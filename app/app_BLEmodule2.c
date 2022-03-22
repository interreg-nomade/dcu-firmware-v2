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

#define PRINTF_APP_BLEMODULE2_DBG			1

static void BLE2Task(const void * params);
static void BLEmodule2_init(void);
int BLEmodule2_Config_Init();

SemaphoreHandle_t wakeBLEmodule2Task;
QueueHandle_t BLEmodule2eventQueue;

static osThreadId BLEmodule2TaskHandle;
static instrument_config_t * pBLEmodule2Instrument;

extern char string[];
extern QueueHandle_t pPrintQueue;

void BLEmodule2_task_init()
{
  osThreadDef(BLEmodule2Task, BLE2Task, osPriorityNormal, 0, 1024);	/* Declaration of BLEmodule task */
  BLEmodule2TaskHandle = osThreadCreate(osThread(BLEmodule2Task), NULL);			/* Start BLEmodule task */
}

static void BLE2Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule2Data;	//Structure to store the BLE module data
  const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
  unsigned int previousQueuing = 0;     //Store the time of the previous post in main queue
  BLEmodule2_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {

	if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
	{/* It is time to send a new message */
	  xSemaphoreTake(wakeBLEmodule2Task, portMAX_DELAY);      // Wait until something happens
	  xQueueReceive(BLEmodule2eventQueue, &sensorEvent, 0);
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule2Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule2Instrument)                              // Time to stream a message
	  {
		imu_100Hz_data_t * pData = NULL;
		pData 				     = (imu_100Hz_data_t*) pBLEmodule2Instrument->data;
		*pData 				     = resBLEmodule2Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLE2Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  previousQueuing = xTaskGetTickCount();
	}
  }
}

static void BLEmodule2_init(void)
{
  BLEmodule2_Config_Init(); // decode configuration and link pointer handler to BLE module 2
  wakeBLEmodule2Task = xSemaphoreCreateBinary(); // Creating binary semaphore
  BLEmodule2eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE2_DBG
  if (BLEmodule2eventQueue == NULL)
  {
	xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLEmodule2_init] Error creating BLE module 2 event queue.\n", 0);
  }
  else
  {
    xQueueSend(pPrintQueue, "[APP_BLEmodule2] [BLEmodule2_init] BLE module 2 event queue created.\n", 0);
  }
#endif
}

void sensorHandlerBLEmodule2(imu_100Hz_data_t *sensorEvent)
{
  xQueueSend(BLEmodule2eventQueue, sensorEvent, 0);
  xSemaphoreGive(wakeBLEmodule2Task);
}

int BLEmodule2_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
//#if PRINTF_APP_BLEMODULE2_DBG
//    sprintf(string, "[APP_BLEmodule2] [BLEmodule2_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
//    xQueueSend(pPrintQueue, string, 0);
//#endif
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
