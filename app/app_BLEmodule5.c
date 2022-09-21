/*
 * app_BLEmodule5.c
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
#include "app_BLEmodule5.h"
#include "common.h"
#include "../config/project_config.h"

#define PRINTF_APP_BLEMODULE5_DBG			1

static void BLE5Task(const void * params);
static void BLEmodule5_init(void);
int BLEmodule5_Config_Init();

SemaphoreHandle_t wakeBLEmodule5Task;
QueueHandle_t BLEmodule5eventQueue;

static osThreadId BLEmodule5TaskHandle;
static instrument_config_t * pBLEmodule5Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint16_t itemsAddedToBLEmodule5eventQueue = 0;
uint16_t itemsSubstractedFromBLEmodule5eventQueue = 0;
//uint64_t timestampqueue5 = 0;
extern uint64_t maxTimeStampInQueues;
extern uint64_t timeStampQueue[6];
extern uint8_t Synchronised;

extern int numberOfModules;

void BLEmodule5_task_init()
{
  osThreadDef(BLEmodule5Task, BLE5Task, osPriorityNormal, 0, 1024);      //Declaration of BLEmodule task
  BLEmodule5TaskHandle = osThreadCreate(osThread(BLEmodule5Task), NULL); // Start BLEmodule task
}

static void BLE5Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule5Data;	//Structure to store the BLE module data
  BLEmodule5_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {
	if (uxQueueMessagesWaiting(BLEmodule5eventQueue))
	{
	  itemsSubstractedFromBLEmodule5eventQueue++;
	  xQueueReceive(BLEmodule5eventQueue, &sensorEvent, 0);
	  timeStampQueue[4] = sensorEvent.timestamp;
      if ((uxQueueMessagesWaiting(BLEmodule5eventQueue) > 10) && ((maxTimeStampInQueues - timeStampQueue[4]) > 19) && (maxTimeStampInQueues > timeStampQueue[4]))
      {
        itemsSubstractedFromBLEmodule5eventQueue++;
        xQueueReceive(BLEmodule5eventQueue, &sensorEvent, 0);
        timeStampQueue[4] = sensorEvent.timestamp;
//        xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLE5Task] removed sensor event to synchronise.\n", 0);
      }
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule5Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule5Instrument)
	  { // Time to stream a message
	    imu_100Hz_data_t * pData = NULL;
	    pData 				   = (imu_100Hz_data_t*) pBLEmodule5Instrument->data;
	    *pData 				   = resBLEmodule5Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLE5Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  osDelay(20);
	}
  }
}

static void BLEmodule5_init(void)
{
  if (BLEmodule5_Config_Init()) // decode configuration and link pointer handler to BLE module 5
  {
    //wakeBLEmodule5Task = xSemaphoreCreateBinary(); // Creating binary semaphore
	switch (numberOfModules)
	{ // Create queue to store data from BLE instrument
	  case 6:
	  {
		BLEmodule5eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 5:
	  {
		BLEmodule5eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_5_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	}
#if PRINTF_APP_BLEMODULE5_DBG
    if (BLEmodule5eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLEmodule5_init] Error creating BLE module 5 event queue.\n", 0);
    }
#endif
  }
}

void sensorHandlerBLEmodule5(imu_100Hz_data_t *sensorEvent)
{
  if (xQueueSend(BLEmodule5eventQueue, sensorEvent, 0) == pdPASS)
  {
    itemsAddedToBLEmodule5eventQueue++;
  }
  else
  { // queue full
//	xQueueSend(pPrintQueue, "[APP_BLEmodule5] [sensorHandlerBLEmodule5] Queue full.\n", 0);
  }
}

int BLEmodule5_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule5Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE5_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLEmodule5_Config_Init] No more BLE modules available, terminate BLE module task 5.\n", 0);
#endif
	osThreadTerminate(BLEmodule5TaskHandle);
	return 0;
  }
}
