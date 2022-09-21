/*
 * app_BLEmodule2.c
 *
 *  Created on: 22 feb. 2022
 *      Author: Sarah Goossens
 */
#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "app_BLEmodule2.h"
#include "common.h"
#include "../config/project_config.h"
//#include "config/raw.h"

#define PRINTF_APP_BLEMODULE2_DBG			1

static void BLE2Task(const void * params);
static void BLEmodule2_init(void);
int BLEmodule2_Config_Init();

SemaphoreHandle_t wakeBLEmodule2Task;
QueueHandle_t BLEmodule2eventQueue;

static osThreadId BLEmodule2TaskHandle;
static instrument_config_t * pBLEmodule2Instrument;

uint16_t itemsAddedToBLEmodule2eventQueue = 0;
uint16_t itemsSubstractedFromBLEmodule2eventQueue = 0;
//uint64_t timestampqueue2 = 0;
extern uint64_t maxTimeStampInQueues;
extern uint64_t timeStampQueue[6];
extern uint8_t Synchronised;

extern char string[];
extern QueueHandle_t pPrintQueue;

extern int numberOfModules;

void BLEmodule2_task_init()
{
  osThreadDef(BLEmodule2Task, BLE2Task, osPriorityNormal, 0, 1024);	/* Declaration of BLEmodule task */
  BLEmodule2TaskHandle = osThreadCreate(osThread(BLEmodule2Task), NULL);			/* Start BLEmodule task */
}

static void BLE2Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule2Data;	//Structure to store the BLE module data
  BLEmodule2_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {
    if (uxQueueMessagesWaiting(BLEmodule2eventQueue))
    {
      itemsSubstractedFromBLEmodule2eventQueue++;
      xQueueReceive(BLEmodule2eventQueue, &sensorEvent, 0);
      timeStampQueue[1] = sensorEvent.timestamp;
      if ((uxQueueMessagesWaiting(BLEmodule2eventQueue) > 10) && ((maxTimeStampInQueues - timeStampQueue[1]) > 19) && (maxTimeStampInQueues > timeStampQueue[1]))
      {
        itemsSubstractedFromBLEmodule2eventQueue++;
        xQueueReceive(BLEmodule2eventQueue, &sensorEvent, 0);
        timeStampQueue[1] = sensorEvent.timestamp;
//        xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLE2Task] removed sensor event to synchronise.\n", 0);
      }
      refreshBLEmoduleData(&sensorEvent, &resBLEmodule2Data); // Copy actual BLE module data to structure that will be queued by reference
      if (pBLEmodule2Instrument)
      { // Time to stream a message
        imu_100Hz_data_t * pData = NULL;
        pData 				   = (imu_100Hz_data_t*) pBLEmodule2Instrument->data;
        *pData 				   = resBLEmodule2Data;
      }
      else
      {
        xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLE2Task] pBLEmoduleInstrument = 0.\n", 0);
      }
      osDelay(20);
    }
  }
}

static void BLEmodule2_init(void)
{
  if (BLEmodule2_Config_Init()) // decode configuration and link pointer handler to BLE module 2
  {
    //wakeBLEmodule2Task = xSemaphoreCreateBinary(); // Creating binary semaphore
	switch (numberOfModules)
	{ // Create queue to store data from BLE instrument
	  case 6:
	  {
	    BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 5:
	  {
	    BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_5_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 4:
	  {
		BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_4_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 3:
	  {
		BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_3_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  case 2:
	  {
		BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE_2_MODULES, sizeof(imu_100Hz_data_t));
		break;
	  }
	  default:
	  {
	    sprintf(string, "%u [APP_BLEmodule2] [BLEmodule2_init] Error creating BLE module 2 event queue, Number of modules is not correct: %d\n",(unsigned int) HAL_GetTick(), (unsigned int)numberOfModules);
	    xQueueSend(pPrintQueue, string, 0);
	  }
	}
#if PRINTF_APP_BLEMODULE2_DBG
    if (BLEmodule2eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLEmodule2_init] Error creating BLE module 2 event queue.\n", 0);
    }
#endif
  }
}

void sensorHandlerBLEmodule2(imu_100Hz_data_t *sensorEvent)
{
  if (xQueueSend(BLEmodule2eventQueue, sensorEvent, 0) == pdPASS)
  {
    itemsAddedToBLEmodule2eventQueue++;
  }
  else
  { // queue full
//	xQueueSend(pPrintQueue, "[APP_BLEmodule2] [sensorHandlerBLEmodule2] Queue full.\n", 0);
  }
}

int BLEmodule2_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule2Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE2_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLEmodule2_Config_Init] No more BLE modules available, terminate BLE module task 2.\n", 0);
#endif
	osThreadTerminate(BLEmodule2TaskHandle);
	return 0;
  }
}
