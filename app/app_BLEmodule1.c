/*
 * app_BLEmodule1.c
 *
 *  Created on: 21 feb. 2022
 *      Author: Sarah Goossens
 */

#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "app_BLEmodule1.h"
#include "common.h"
#include "../config/project_config.h"

#define PRINTF_APP_BLEMODULE1_DBG			1

//Number of instruments

static void BLE1Task(const void * params);
static void BLEmodule1_init(void);
int BLEmodule1_Config_Init();

SemaphoreHandle_t wakeBLEmodule1Task;
QueueHandle_t BLEmodule1eventQueue;

static osThreadId BLEmodule1TaskHandle;
static instrument_config_t * pBLEmodule1Instrument;

uint16_t itemsAddedToBLEmodule1eventQueue = 0;
uint16_t itemsSubstractedFromBLEmodule1eventQueue = 0;
//uint64_t timestampqueue1 = 0;
extern uint64_t maxTimeStampInQueues;
extern uint64_t timeStampQueue[6];
extern uint8_t Synchronised;

extern char string[];
extern QueueHandle_t pPrintQueue;

extern int numberOfModules;

void BLEmodule1_task_init()
{
  osThreadDef(BLEmodule1Task, BLE1Task, osPriorityNormal, 0, 1024);	/* Declaration of BLEmodule1 task */
  BLEmodule1TaskHandle = osThreadCreate(osThread(BLEmodule1Task), NULL);			/* Start BLEmodule task */
}

static void BLE1Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule1Data;	//Structure to store the BLE module data
  BLEmodule1_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {
    if (uxQueueMessagesWaiting(BLEmodule1eventQueue))
    {
      itemsSubstractedFromBLEmodule1eventQueue++;
      xQueueReceive(BLEmodule1eventQueue, &sensorEvent, 0);
      timeStampQueue[0] = sensorEvent.timestamp;
      if ((uxQueueMessagesWaiting(BLEmodule1eventQueue) > 10) && ((maxTimeStampInQueues - timeStampQueue[0]) > 19) && (maxTimeStampInQueues > timeStampQueue[0]))
      {
        itemsSubstractedFromBLEmodule1eventQueue++;
        xQueueReceive(BLEmodule1eventQueue, &sensorEvent, 0);
        timeStampQueue[0] = sensorEvent.timestamp;
//        xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLE1Task] removed sensor event to synchronise.\n", 0);
      }
      refreshBLEmoduleData(&sensorEvent, &resBLEmodule1Data);  // Copy actual BLE module data to structure that will be queued by reference
      if (pBLEmodule1Instrument)
      { // Time to stream a message
        imu_100Hz_data_t * pData = NULL;
        pData 				   = (imu_100Hz_data_t*) pBLEmodule1Instrument->data;
        *pData 				   = resBLEmodule1Data;
      }
      else
      {
        xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLE1Task] pBLEmoduleInstrument = 0.\n", 0);
      }
      osDelay(20);
    }
  }
}

static void BLEmodule1_init(void)
{
  if (BLEmodule1_Config_Init())
  { // configuration is decoded and pointer handler is linked to BLE module 1
	switch (numberOfModules)
	{ // Create queue to store data from BLE instrument
	  case 6:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 5:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_5_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 4:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_4_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 3:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_3_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 2:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_2_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 1:
	  {
		BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_1_MODULE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
		break;
	  }
	  default:
	  {
		xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_init] Error creating BLE module 1 event queue, Number of modules is not correct.\n", 0);
	  }
	}
#if PRINTF_APP_BLEMODULE1_DBG
    if (BLEmodule1eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_init] Error creating BLE module 1 event queue.\n", 0);
    }
#endif
  }
  else
  {
    xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_init] BLE module 1 event queue not created.\n", 0);
  }
}

void sensorHandlerBLEmodule1(imu_100Hz_data_t *sensorEvent)
{
  if (xQueueSend(BLEmodule1eventQueue, sensorEvent, 0) == pdPASS)
  {
    itemsAddedToBLEmodule1eventQueue++;
  }
//  else
//  { // queue full
//	xQueueSend(pPrintQueue, "[APP_BLEmodule1] [sensorHandlerBLEmodule1] Queue full.\n", 0);
//  }
}

int BLEmodule1_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t struct from decodedConfig struct
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule1Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE1_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_Config_Init] No more BLE modules available, terminate BLE module task 1.\n", 0);
#endif
	osThreadTerminate(BLEmodule1TaskHandle);
	return 0;
  }
}
