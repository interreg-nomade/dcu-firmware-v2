/*
 * app_BLEmodule6.c
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
#include "app_BLEmodule6.h"
#include "common.h"
#include "../config/project_config.h"

#define PRINTF_APP_BLEMODULE6_DBG			1

static void BLE6Task(const void * params);
static void BLEmodule6_init(void);
int BLEmodule6_Config_Init();

SemaphoreHandle_t wakeBLEmodule6Task;
QueueHandle_t BLEmodule6eventQueue;

static osThreadId BLEmodule6TaskHandle;
static instrument_config_t * pBLEmodule6Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint16_t itemsAddedToBLEmodule6eventQueue = 0;
uint16_t itemsSubstractedFromBLEmodule6eventQueue = 0;
//uint64_t timestampqueue6 = 0;
extern uint64_t maxTimeStampInQueues;
extern uint64_t timeStampQueue[6];
extern uint8_t Synchronised;


void BLEmodule6_task_init()
{
  osThreadDef(BLEmodule6Task, BLE6Task, osPriorityNormal, 0, 1024);      //Declaration of BLEmodule task
  BLEmodule6TaskHandle = osThreadCreate(osThread(BLEmodule6Task), NULL); // Start BLEmodule task
}

static void BLE6Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule6Data;	//Structure to store the BLE module data
  BLEmodule6_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {
	if (uxQueueMessagesWaiting(BLEmodule6eventQueue))
	{
	  itemsSubstractedFromBLEmodule6eventQueue++;
	  xQueueReceive(BLEmodule6eventQueue, &sensorEvent, 0);
	  timeStampQueue[5] = sensorEvent.timestamp;
      if ((uxQueueMessagesWaiting(BLEmodule6eventQueue) > 10) && ((maxTimeStampInQueues - timeStampQueue[5]) > 19) && (maxTimeStampInQueues > timeStampQueue[5]))
      {
        itemsSubstractedFromBLEmodule6eventQueue++;
        xQueueReceive(BLEmodule6eventQueue, &sensorEvent, 0);
        timeStampQueue[5] = sensorEvent.timestamp;
//        xQueueSend(pPrintQueue, "[APP_BLEmodule6] [BLE6Task] removed sensor event to synchronise.\n", 0);
      }
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule6Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule6Instrument)
	  { // Time to stream a message
	    imu_100Hz_data_t * pData = NULL;
	    pData 				   = (imu_100Hz_data_t*) pBLEmodule6Instrument->data;
	    *pData 				   = resBLEmodule6Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule6] [BLE6Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  osDelay(20);
	}
  }
}

static void BLEmodule6_init(void)
{
  if (BLEmodule6_Config_Init()) // decode configuration and link pointer handler to BLE module 6
  {
    BLEmodule6eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE6_DBG
    if (BLEmodule6eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule6] [BLEmodule6_init] Error creating BLE module 6 event queue.\n", 0);
    }
#endif
  }
}

void sensorHandlerBLEmodule6(imu_100Hz_data_t *sensorEvent)
{
  if (xQueueSend(BLEmodule6eventQueue, sensorEvent, 0) == pdPASS)
  {
    itemsAddedToBLEmodule6eventQueue++;
  }
  else
  { // queue full
//	xQueueSend(pPrintQueue, "[APP_BLEmodule6] [sensorHandlerBLEmodule6] Queue full.\n", 0);
  }
}

int BLEmodule6_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule6Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE6_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule6] [BLEmodule6_Config_Init] No more BLE modules available, terminate BLE module task 6.\n", 0);
#endif
	osThreadTerminate(BLEmodule6TaskHandle);
	return 0;
  }
}
