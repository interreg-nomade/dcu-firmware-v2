/*
 * app_BLEmodule3.c
 *
 *  Created on: 23 feb. 2022
 *      Author: Sarah Goossens
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "app_BLEmodule3.h"
#include "common.h"
#include "../config/project_config.h"

#define PRINTF_APP_BLEMODULE3_DBG			1

static void BLE3Task(const void * params);
static void BLEmodule3_init(void);
int BLEmodule3_Config_Init();

SemaphoreHandle_t wakeBLEmodule3Task;
QueueHandle_t BLEmodule3eventQueue;

static osThreadId BLEmodule3TaskHandle;
static instrument_config_t * pBLEmodule3Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint16_t itemsAddedToBLEmodule3eventQueue = 0;
uint16_t itemsSubstractedFromBLEmodule3eventQueue = 0;
//uint64_t timestampqueue3 = 0;
extern uint64_t maxTimeStampInQueues;
extern uint64_t timeStampQueue[6];
extern uint8_t Synchronised;

extern int numberOfModules;

void BLEmodule3_task_init()
{
  osThreadDef(BLEmodule3Task, BLE3Task, osPriorityNormal, 0, 1024);      //Declaration of BLEmodule task
  BLEmodule3TaskHandle = osThreadCreate(osThread(BLEmodule3Task), NULL); // Start BLEmodule task
}

static void BLE3Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule3Data;	//Structure to store the BLE module data
  BLEmodule3_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {
    if (uxQueueMessagesWaiting(BLEmodule3eventQueue))
    {
      itemsSubstractedFromBLEmodule3eventQueue++;
      xQueueReceive(BLEmodule3eventQueue, &sensorEvent, 0);
      timeStampQueue[2] = sensorEvent.timestamp;
      if ((uxQueueMessagesWaiting(BLEmodule3eventQueue) > 10) && ((maxTimeStampInQueues - timeStampQueue[2]) > 19) && (maxTimeStampInQueues > timeStampQueue[2]))
      {
        itemsSubstractedFromBLEmodule3eventQueue++;
        xQueueReceive(BLEmodule3eventQueue, &sensorEvent, 0);
        timeStampQueue[2] = sensorEvent.timestamp;
//        xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLE3Task] removed sensor event to synchronise.\n", 0);
      }
      refreshBLEmoduleData(&sensorEvent, &resBLEmodule3Data); // Copy actual BLE module data to structure that will be queued by reference
      if (pBLEmodule3Instrument)
      { // Time to stream a message
        imu_100Hz_data_t * pData = NULL;
        pData 				   = (imu_100Hz_data_t*) pBLEmodule3Instrument->data;
        *pData 				   = resBLEmodule3Data;
      }
      else
      {
        xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLE3Task] pBLEmoduleInstrument = 0.\n", 0);
      }
      osDelay(20);
    }
  }
}

static void BLEmodule3_init(void)
{
  if (BLEmodule3_Config_Init()) // decode configuration and link pointer handler to BLE module 3
  {
    //wakeBLEmodule3Task = xSemaphoreCreateBinary(); // Creating binary semaphore
	switch (numberOfModules)
	{ // Create queue to store data from BLE instrument
	  case 6:
	  {
		BLEmodule3eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 5:
	  {
		BLEmodule3eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_5_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 4:
	  {
		BLEmodule3eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_4_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 3:
	  {
		BLEmodule3eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_3_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  default:
	  {
	    sprintf(string, "%u [APP_BLEmodule3] [BLEmodule3_init] Error creating BLE module 3 event queue, Number of modules is not correct: %d\n",(unsigned int) HAL_GetTick(), (unsigned int)numberOfModules);
	    xQueueSend(pPrintQueue, string, 0);
	  }
	}
#if PRINTF_APP_BLEMODULE3_DBG
    if (BLEmodule3eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLEmodule3_init] Error creating BLE module 3 event queue.\n", 0);
    }
#endif
  }
}

void sensorHandlerBLEmodule3(imu_100Hz_data_t *sensorEvent)
{
  if (xQueueSend(BLEmodule3eventQueue, sensorEvent, 0) == pdPASS)
  {
    itemsAddedToBLEmodule3eventQueue++;
  }
  else
  { // queue full
//	xQueueSend(pPrintQueue, "[APP_BLEmodule3] [sensorHandlerBLEmodule3] Queue full.\n", 0);
  }
  //xSemaphoreGive(wakeBLEmodule3Task);
}

int BLEmodule3_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
//#if PRINTF_APP_BLEMODULE3_DBG
//    sprintf(string, "[APP_BLEmodule3] [BLEmodule3_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
//    xQueueSend(pPrintQueue, string, 0);
//#endif
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule3Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE3_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLEmodule3_Config_Init] No more BLE modules available, terminate BLE module task 3.\n", 0);
#endif
	osThreadTerminate(BLEmodule3TaskHandle);
	return 0;
  }
}
