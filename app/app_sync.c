/**
 * @file app_sync.c
 * @brief Synchronization between storage and streaming
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 */

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "config/config_op.h"

#include "app_streamer.h"
#include "app_storage.h"
#include "app_rtc.h"
#include "app_sync.h"
#include "usart.h"

#include "gpio.h"
#include "main.h"
#include "common.h"

#include "queues/streamer_service/streamer_service_queue.h"

#include "config/raw.h"


#define PRINT_APP_SYNC_DBG_MSG						1
#define PRINTF_APP_SYNC								1

void syncThread(const void * params);
static osThreadId syncThreadHandler;

extern char string[];
extern QueueHandle_t pPrintQueue;

extern streamerServiceQueueMsg_t streamMsgEnabled;

extern int numberOfModules;

uint64_t timeStampQueue[6] = {0,0,0,0,0,0};
uint64_t maxTimeStampInQueues = 0;
uint8_t streamPacket = 0;
uint8_t Synchronised = 0;

extern uint8_t noPacketStreamed;

extern QueueHandle_t BLEmodule1eventQueue;
extern uint16_t itemsAddedToBLEmodule1eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule1eventQueue;
//extern uint64_t timestampqueue1;

extern QueueHandle_t BLEmodule2eventQueue;
extern uint16_t itemsAddedToBLEmodule2eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule2eventQueue;
//extern uint64_t timestampqueue2;

extern QueueHandle_t BLEmodule3eventQueue;
extern uint16_t itemsAddedToBLEmodule3eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule3eventQueue;
//extern uint64_t timestampqueue3;

extern QueueHandle_t BLEmodule4eventQueue;
extern uint16_t itemsAddedToBLEmodule4eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule4eventQueue;
//extern uint64_t timestampqueue4;

extern QueueHandle_t BLEmodule5eventQueue;
extern uint16_t itemsAddedToBLEmodule5eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule5eventQueue;
//extern uint64_t timestampqueue5;

extern QueueHandle_t BLEmodule6eventQueue;
extern uint16_t itemsAddedToBLEmodule6eventQueue;
extern uint16_t itemsSubstractedFromBLEmodule6eventQueue;
//extern uint64_t timestampqueue6;

extern uint64_t epochMeasurementStart; // epoch time when measurement started, see app_tablet_com.c

void initSyncThread()
{
  osThreadDef(syncThread, syncThread, osPriorityAboveNormal, 0, 256); /* SyncThread definition  */
  syncThreadHandler = osThreadCreate (osThread(syncThread), NULL);    /* Start SyncThread  */
}

void syncThread(const void * params)
{
  uint8_t Queuedepth1 = 0;
  uint8_t Queuedepth2 = 0;
  uint8_t Queuedepth3 = 0;
  uint8_t Queuedepth4 = 0;
  uint8_t Queuedepth5 = 0;
  uint8_t Queuedepth6 = 0;
  uint8_t QueuedepthCheck = 16;
  uint8_t QueuedepthCheckMin = 0;
  uint8_t MaxValueLocation = 0;

//  switch (numberOfModules)
//  {
//    case 6:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE - 19;
//	  break;
//    }
//    case 5:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE_5_MODULES - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE_5_MODULES - 19;
//	  break;
//    }
//    case 4:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE_4_MODULES - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE_4_MODULES - 19;
//	  break;
//    }
//    case 3:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE_3_MODULES - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE_3_MODULES - 19;
//	  break;
//    }
//    case 2:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE_2_MODULES - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE_2_MODULES - 19;
//	  break;
//    }
//    case 1:
//    {
//	  QueuedepthCheck = SENSOR_EVENT_QUEUE_SIZE_1_MODULE - 1;
//	  QueuedepthCheckMin = SENSOR_EVENT_QUEUE_SIZE_1_MODULE - 19;
//	  break;
//    }
//    default:
//    {
//	  xQueueSend(pPrintQueue, "[app_syc] Error defining QueuedepthCheck.\n", 0);
//    }
//  }
  while (decodedConfig.state != CONF_CORRECT)
  {  /* Loop until the configuration is decoded */
	osDelay(20);
  }

  uint32_t ulInterruptStatus;
  xTaskNotifyStateClear(NULL); /* Clear the notification before entering the while loop */
  uint8_t printSynchronised = 0;
  uint32_t formerCycleCounter = app_rtc_get_cycle_counter();
  uint32_t lastTimeOfStreamerNotification = 0;
  uint8_t firstTimeSynchronised = 0;
  BaseType_t xResult;
  for(;;)
  {
	/* Block until we get the notification from the timer interrupt
	 * See tim2_callback() in timer_callback.c, which is calling app_sync_notify_from_isr() every 20ms
	 */
//	  osDelay(20);
	xResult = xTaskNotifyWait( 0x00,  /* Don't clear any bits on entry. */
	          0xffffffff,             /* Clear all bits on exit. (long max) */
	          &ulInterruptStatus,     /* Receives the notification value. */
	          pdMS_TO_TICKS(25) );    /* Block task max for 25ms */
//	if (xTaskNotifyWait( 0x00,                          /* Don't clear any bits on entry. */
//				         0xffffffff,                    /* Clear all bits on exit. (long max) */
//				         &ulInterruptStatus,            /* Receives the notification value. */
//				         portMAX_DELAY ) == pdTRUE)     /* Block task indefinitely.  */
    if (xResult == pdPASS)
	{ /* Every 20ms, snapshot (copy) the configuration and copy decodedConfig in snapshotConfig */
	  if (noPacketStreamed)
	  {
		sprintf(string, "%u [app_syc] No packet streamed for Cycle Counter: %08X. Last streamer notification was at: %u. Difference is %ums.\n",(unsigned int) HAL_GetTick(),
			(unsigned int) decodedConfig.conf.cycleCounter + 1,
			(unsigned int) lastTimeOfStreamerNotification,
			(unsigned int) (HAL_GetTick() - lastTimeOfStreamerNotification));
		    xQueueSend(pPrintQueue, string, 0);
		    noPacketStreamed = 0;
	  }
	  decodedConfig.get(); 												// Get the mutex of the decoded config
	  decodedConfig.conf.cycleCounter = app_rtc_get_cycle_counter(); 	// Get the cycle counter and store it in the decoded config
      if ((decodedConfig.conf.cycleCounter - formerCycleCounter) > 1)
      {
    	printSynchronised = 0;
      }
	  formerCycleCounter = app_rtc_get_cycle_counter();
	  config_copy(&snapshotconf, &decodedConfig.conf); 					// Make a copy of the decoded config
	  decodedConfig.release(); 											// Release the mutex of the decoded config
	  app_streamer_notify(APP_STREAMER_NOTIF_CYCLE_COUNTER);  			// Notify the streamer thread
	  lastTimeOfStreamerNotification = HAL_GetTick();
	  //app_storage_notify(APP_STORAGE_NOTIF_CYCLE_COUNTER);				// Notify the storage thread
  	  MaxValueLocation = 0;
  	  for (int i = 1; i < 6; i++)
  	  { // find the max value of the received time stamps:
  	    if (timeStampQueue[i] > timeStampQueue[MaxValueLocation])
  	    {
	      MaxValueLocation = i;
  	    }
  	  }
  	  maxTimeStampInQueues = timeStampQueue[MaxValueLocation];
  	  streamPacket = 0;
#if PRINTF_APP_SYNC
  	  if (streamMsgEnabled.action == streamerService_EnableAndroidStream)
      {
 	  	switch (numberOfModules)
  	    {
	  	  case 6:
	 	  {
   	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
   	  		Queuedepth2 = (unsigned int) uxQueueMessagesWaiting(BLEmodule2eventQueue);
   	  		Queuedepth3 = (unsigned int) uxQueueMessagesWaiting(BLEmodule3eventQueue);
   	  		Queuedepth4 = (unsigned int) uxQueueMessagesWaiting(BLEmodule4eventQueue);
   	  		Queuedepth5 = (unsigned int) uxQueueMessagesWaiting(BLEmodule5eventQueue);
   	  		Queuedepth6 = (unsigned int) uxQueueMessagesWaiting(BLEmodule6eventQueue);
   		    if (Queuedepth1 > QueuedepthCheck || Queuedepth2 > QueuedepthCheck || Queuedepth3 > QueuedepthCheck || Queuedepth4 > QueuedepthCheck || Queuedepth5 > QueuedepthCheck || Queuedepth6 > QueuedepthCheck)
   		    {
              if((printSynchronised < 1) && timeStampQueue[0] && timeStampQueue[1] && timeStampQueue[2] && timeStampQueue[3] && timeStampQueue[4] && timeStampQueue[5] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20) && ((maxTimeStampInQueues - timeStampQueue[1]) < 20) && ((maxTimeStampInQueues - timeStampQueue[2]) < 20) && ((maxTimeStampInQueues - timeStampQueue[3]) < 20) && ((maxTimeStampInQueues - timeStampQueue[4]) < 20) && ((maxTimeStampInQueues - timeStampQueue[5]) < 20))
              {
            	printSynchronised++;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
	                (unsigned int) decodedConfig.conf.cycleCounter,
   	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
   		            Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
   		            Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
   		            Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
   		            Queuedepth5, (unsigned int) (timeStampQueue[4] - epochMeasurementStart),
   		            Queuedepth6, (unsigned int) (timeStampQueue[5] - epochMeasurementStart),
		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                firstTimeSynchronised = 1;
                Synchronised = 1;
  	          }
              if(firstTimeSynchronised && (((maxTimeStampInQueues - timeStampQueue[0]) > 19) || ((maxTimeStampInQueues - timeStampQueue[1]) > 19) || ((maxTimeStampInQueues - timeStampQueue[2]) > 19) || ((maxTimeStampInQueues - timeStampQueue[3]) > 19) || ((maxTimeStampInQueues - timeStampQueue[4]) > 19) || ((maxTimeStampInQueues - timeStampQueue[5]) > 19)))
              {
             	printSynchronised = 0;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
   		            (unsigned int) decodedConfig.conf.cycleCounter,
     	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
       		        Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
       		        Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
       		        Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
       		        Queuedepth5, (unsigned int) (timeStampQueue[4] - epochMeasurementStart),
       		        Queuedepth6, (unsigned int) (timeStampQueue[5] - epochMeasurementStart),
   		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                Synchronised = 0;
              }
   	 	    }
   		    break;
	 	  }
  	  	  case 5:
  	 	  {
   	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
   	  		Queuedepth2 = (unsigned int) uxQueueMessagesWaiting(BLEmodule2eventQueue);
   	  		Queuedepth3 = (unsigned int) uxQueueMessagesWaiting(BLEmodule3eventQueue);
   	  		Queuedepth4 = (unsigned int) uxQueueMessagesWaiting(BLEmodule4eventQueue);
   	  		Queuedepth5 = (unsigned int) uxQueueMessagesWaiting(BLEmodule5eventQueue);
   		    if (Queuedepth1 > QueuedepthCheck || Queuedepth2 > QueuedepthCheck || Queuedepth3 > QueuedepthCheck || Queuedepth4 > QueuedepthCheck || Queuedepth5 > QueuedepthCheck)
   		    {
              if((printSynchronised < 1) && timeStampQueue[0] && timeStampQueue[1] && timeStampQueue[2] && timeStampQueue[3] && timeStampQueue[4] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20) && ((maxTimeStampInQueues - timeStampQueue[1]) < 20) && ((maxTimeStampInQueues - timeStampQueue[2]) < 20) && ((maxTimeStampInQueues - timeStampQueue[3]) < 20) && ((maxTimeStampInQueues - timeStampQueue[4]) < 20))
              {
            	printSynchronised++;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
	                (unsigned int) decodedConfig.conf.cycleCounter,
   	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
   		            Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
   		            Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
   		            Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
   		            Queuedepth5, (unsigned int) (timeStampQueue[4] - epochMeasurementStart),
		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                firstTimeSynchronised = 1;
                Synchronised = 1;
  	          }
              if(firstTimeSynchronised && (((maxTimeStampInQueues - timeStampQueue[0]) > 19) || ((maxTimeStampInQueues - timeStampQueue[1]) > 19) || ((maxTimeStampInQueues - timeStampQueue[2]) > 19) || ((maxTimeStampInQueues - timeStampQueue[3]) > 19) || ((maxTimeStampInQueues - timeStampQueue[4]) > 19)))
              {
             	printSynchronised = 0;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
   		            (unsigned int) decodedConfig.conf.cycleCounter,
     	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
       		        Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
       		        Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
       		        Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
       		        Queuedepth5, (unsigned int) (timeStampQueue[4] - epochMeasurementStart),
   		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                Synchronised = 0;
              }
   	 	    }
   		    break;
  	 	  }
  	  	  case 4:
  	 	  {
   	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
   	  		Queuedepth2 = (unsigned int) uxQueueMessagesWaiting(BLEmodule2eventQueue);
   	  		Queuedepth3 = (unsigned int) uxQueueMessagesWaiting(BLEmodule3eventQueue);
   	  		Queuedepth4 = (unsigned int) uxQueueMessagesWaiting(BLEmodule4eventQueue);
   		    if (Queuedepth1 > QueuedepthCheck || Queuedepth2 > QueuedepthCheck || Queuedepth3 > QueuedepthCheck  || Queuedepth4 > QueuedepthCheck)
   		    {
              if((printSynchronised < 1) && timeStampQueue[0] && timeStampQueue[1] && timeStampQueue[2] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20) && ((maxTimeStampInQueues - timeStampQueue[1]) < 20) && ((maxTimeStampInQueues - timeStampQueue[2]) < 20) && ((maxTimeStampInQueues - timeStampQueue[3]) < 20))
              {
             	printSynchronised++;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
 		            (unsigned int) decodedConfig.conf.cycleCounter,
   	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
   		            Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
   		            Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
   		            Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
 		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                firstTimeSynchronised = 1;
                Synchronised = 1;
 	          }
              if(firstTimeSynchronised && (((maxTimeStampInQueues - timeStampQueue[0]) > 19) || ((maxTimeStampInQueues - timeStampQueue[1]) > 19) || ((maxTimeStampInQueues - timeStampQueue[2]) > 19) || ((maxTimeStampInQueues - timeStampQueue[3]) > 19)))
              {
             	printSynchronised = 0;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
   		            (unsigned int) decodedConfig.conf.cycleCounter,
     	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
     		        Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
     		        Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
     		        Queuedepth4, (unsigned int) (timeStampQueue[3] - epochMeasurementStart),
   		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                Synchronised = 0;
              }
   	 	    }
   		    break;
  	 	  }
  	  	  case 3:
  	 	  {
  	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
  	  		Queuedepth2 = (unsigned int) uxQueueMessagesWaiting(BLEmodule2eventQueue);
  	  		Queuedepth3 = (unsigned int) uxQueueMessagesWaiting(BLEmodule3eventQueue);
  		    if (Queuedepth1 > QueuedepthCheck || Queuedepth2 > QueuedepthCheck || Queuedepth3 > QueuedepthCheck)
  		    {
              if((printSynchronised < 1) && timeStampQueue[0] && timeStampQueue[1] && timeStampQueue[2] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20) && ((maxTimeStampInQueues - timeStampQueue[1]) < 20) && ((maxTimeStampInQueues - timeStampQueue[2]) < 20))
              {
            	printSynchronised++;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
		            (unsigned int) decodedConfig.conf.cycleCounter,
  	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
  		            Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
  		            Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                firstTimeSynchronised = 1;
                Synchronised = 1;
	          }
              if(firstTimeSynchronised && (((maxTimeStampInQueues - timeStampQueue[0]) > 19) || ((maxTimeStampInQueues - timeStampQueue[1]) > 19) || ((maxTimeStampInQueues - timeStampQueue[2]) > 19)))
              {
            	printSynchronised = 0;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
  		            (unsigned int) decodedConfig.conf.cycleCounter,
    	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
    		        Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
    		        Queuedepth3, (unsigned int) (timeStampQueue[2] - epochMeasurementStart),
  		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                Synchronised = 0;
              }
  	 	    }
  		    break;
  	 	  }
  	  	  case 2:
  	 	  {
   	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
   	  		Queuedepth2 = (unsigned int) uxQueueMessagesWaiting(BLEmodule2eventQueue);
   		    if (Queuedepth1 > QueuedepthCheck || Queuedepth2 > QueuedepthCheck)
   		    {
               if((printSynchronised < 1) && timeStampQueue[0] && timeStampQueue[1] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20) && ((maxTimeStampInQueues - timeStampQueue[1]) < 20))
               {
             	printSynchronised++;
                 sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
 		            (unsigned int) decodedConfig.conf.cycleCounter,
   	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
   		            Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
 		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                 xQueueSend(pPrintQueue, string, 0);
                 firstTimeSynchronised = 1;
                 Synchronised = 1;
 	          }
               if(firstTimeSynchronised && (((maxTimeStampInQueues - timeStampQueue[0]) > 19) || ((maxTimeStampInQueues - timeStampQueue[1]) > 19)))
               {
             	printSynchronised = 0;
                 sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
   		            (unsigned int) decodedConfig.conf.cycleCounter,
     	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
     		        Queuedepth2, (unsigned int) (timeStampQueue[1] - epochMeasurementStart),
   		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                 xQueueSend(pPrintQueue, string, 0);
                 Synchronised = 0;
               }
   	 	    }
   		    break;
  	 	  }
  	  	  case 1:
  	 	  {
   	  		Queuedepth1 = (unsigned int) uxQueueMessagesWaiting(BLEmodule1eventQueue);
   		    if (Queuedepth1 > QueuedepthCheck)
   		    {
              if((printSynchronised < 1) && timeStampQueue[0] && ((maxTimeStampInQueues - timeStampQueue[0]) < 20))
              {
             	printSynchronised++;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|Max = %u|Synchronised\n",(unsigned int) HAL_GetTick(),
	                (unsigned int) decodedConfig.conf.cycleCounter,
  	                Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
 		            (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                firstTimeSynchronised = 1;
                Synchronised = 1;
   	          }
              if(firstTimeSynchronised && ((maxTimeStampInQueues - timeStampQueue[0]) > 19))
              {
               	printSynchronised = 0;
                sprintf(string, "%u [app_syc] Cycle Counter: %08X, QD - TS|%02X - %u|Max = %u|Out of sync\n",(unsigned int) HAL_GetTick(),
     		        (unsigned int) decodedConfig.conf.cycleCounter,
       	            Queuedepth1, (unsigned int) (timeStampQueue[0] - epochMeasurementStart),
     		        (unsigned int) (maxTimeStampInQueues - epochMeasurementStart));
                xQueueSend(pPrintQueue, string, 0);
                Synchronised = 0;
              }
     	 	}
     		break;
  	 	  }
  		}
 	  	maxTimeStampInQueues += 20;
      }
#endif
	}
    else
    {
		sprintf(string, "%u [app_syc] xResult = pdFALSE, No packet streamed for Cycle Counter: %08X. Last streamer notification was at: %u. Difference is %ums.\n",(unsigned int) HAL_GetTick(),
			(unsigned int) decodedConfig.conf.cycleCounter + 1,
			(unsigned int) lastTimeOfStreamerNotification,
			(unsigned int) (HAL_GetTick() - lastTimeOfStreamerNotification));
		    xQueueSend(pPrintQueue, string, 0);
		    noPacketStreamed = 0;
    }
  }
}

void app_sync_notify_from_isr(uint32_t notValue)
{
  if (osKernelRunning())
  {
	BaseType_t xHigherPriorityTaskWoken;
	BaseType_t res;
	xHigherPriorityTaskWoken = pdFALSE;
	if (syncThreadHandler != NULL)
	{
	  res = xTaskNotifyFromISR( syncThreadHandler, notValue, eSetBits, &xHigherPriorityTaskWoken );
	  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
  }
}
