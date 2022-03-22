/*
 * app_BLEmodule4.c
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
#include "app_BLEmodule4.h"
#include "common.h"
#include "../config/project_config.h"

#define PRINTF_APP_BLEMODULE4_DBG			1

static void BLE4Task(const void * params);
static void BLEmodule4_init(void);
int BLEmodule4_Config_Init();

SemaphoreHandle_t wakeBLEmodule4Task;
QueueHandle_t BLEmodule4eventQueue;

static osThreadId BLEmodule4TaskHandle;
static instrument_config_t * pBLEmodule4Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

void BLEmodule4_task_init()
{
  osThreadDef(BLEmodule4Task, BLE4Task, osPriorityNormal, 0, 1024); //Declaration of BLEmodule task
  BLEmodule4TaskHandle = osThreadCreate(osThread(BLEmodule4Task), NULL); // Start BLEmodule task
}

static void BLE4Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule4Data;	//Structure to store the BLE module data
  const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
  unsigned int previousQueuing = 0;     //Store the time of the previous post in main queue
  BLEmodule4_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {

	if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
	{/* It is time to send a new message */
	  xSemaphoreTake(wakeBLEmodule4Task, portMAX_DELAY);      // Wait until something happens
	  xQueueReceive(BLEmodule4eventQueue, &sensorEvent, 0);
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule4Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule4Instrument)                              // Time to stream a message
	  {
		imu_100Hz_data_t * pData = NULL;
		pData 				     = (imu_100Hz_data_t*) pBLEmodule4Instrument->data;
		*pData 				     = resBLEmodule4Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule4] [BLE4Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  previousQueuing = xTaskGetTickCount();
	}
  }
}

static void BLEmodule4_init(void)
{
  BLEmodule4_Config_Init(); // decode configuration and link pointer handler to BLE module 4
  wakeBLEmodule4Task = xSemaphoreCreateBinary(); // Creating binary semaphore
  BLEmodule4eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE4_DBG
  if (BLEmodule4eventQueue == NULL)
  {
	xQueueSend(pPrintQueue, "[APP_BLEmodule4] [BLEmodule4_init] Error creating BLE module 4 event queue.\n", 0);
  }
  else
  {
    xQueueSend(pPrintQueue, "[APP_BLEmodule4] [BLEmodule4_init] BLE module 4 event queue created.\n", 0);
  }
#endif
}

void sensorHandlerBLEmodule4(imu_100Hz_data_t *sensorEvent)
{
  xQueueSend(BLEmodule4eventQueue, sensorEvent, 0);
  xSemaphoreGive(wakeBLEmodule4Task);
}

int BLEmodule4_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
//#if PRINTF_APP_BLEMODULE4_DBG
//    sprintf(string, "[APP_BLEmodule4] [BLEmodule4_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
//    xQueueSend(pPrintQueue, string, 0);
//#endif
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule4Instrument, SETUP_PRM_COMM_METHOD_BT);
  }
  else
  {
#if PRINTF_APP_BLEMODULE4_DBG
    xQueueSend(pPrintQueue, "[APP_BLEmodule4] [BLEmodule4_Config_Init] No more BLE modules available, terminate BLE module task 4.\n", 0);
#endif
	osThreadTerminate(BLEmodule4TaskHandle);
	return 0;
  }
}
