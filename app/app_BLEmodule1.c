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

static void BLE1Task(const void * params);
static void BLEmodule1_init(void);
int BLEmodule1_Config_Init();

SemaphoreHandle_t wakeBLEmodule1Task;
QueueHandle_t BLEmodule1eventQueue;

static osThreadId BLEmodule1TaskHandle;
static instrument_config_t * pBLEmodule1Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

void BLEmodule1_task_init()
{
  osThreadDef(BLEmodule1Task, BLE1Task, osPriorityNormal, 0, 1024);	/* Declaration of BLEmodule1 task */
  BLEmodule1TaskHandle = osThreadCreate(osThread(BLEmodule1Task), NULL);			/* Start BLEmodule task */
}

static void BLE1Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule1Data;	//Structure to store the BLE module data
  const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
  unsigned int previousQueuing = 0;     //Store the time of the previous post in main queue
  BLEmodule1_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {

	if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
	{/* It is time to send a new message */
	  xSemaphoreTake(wakeBLEmodule1Task, portMAX_DELAY);  // Wait until something happens
	  xQueueReceive(BLEmodule1eventQueue, &sensorEvent, 0);
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule1Data);  // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule1Instrument) // Time to stream a message
	  {
		imu_100Hz_data_t * pData = NULL;
		pData 				     = (imu_100Hz_data_t*) pBLEmodule1Instrument->data;
		*pData 				     = resBLEmodule1Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLE1Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  previousQueuing = xTaskGetTickCount();
	}
  }
}

static void BLEmodule1_init(void)
{
  if (BLEmodule1_Config_Init())
  { // configuration is decoded and pointer handler is linked to BLE module 1
    wakeBLEmodule1Task = xSemaphoreCreateBinary(); // Creating binary semaphore
    BLEmodule1eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE1_DBG
    if (BLEmodule1eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_init] Error creating BLE module 1 event queue.\n", 0);
    }
    else
    {
      xQueueSend(pPrintQueue, "[APP_BLEmodule1] [BLEmodule1_init] BLE module 1 event queue created.\n", 0);
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
  xQueueSend(BLEmodule1eventQueue, sensorEvent, 0);
  xSemaphoreGive(wakeBLEmodule1Task);
}

int BLEmodule1_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t struct from decodedConfig struct
//#if PRINTF_APP_BLEMODULE1_DBG
//    sprintf(string, "[APP_BLEmodule1] [BLEmodule1_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
//    xQueueSend(pPrintQueue, string, 0);
//#endif
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
