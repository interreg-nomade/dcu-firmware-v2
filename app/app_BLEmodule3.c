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

void BLEmodule3_task_init()
{
  osThreadDef(BLEmodule3Task, BLE3Task, osPriorityNormal, 0, 1024); //Declaration of BLEmodule task
  BLEmodule3TaskHandle = osThreadCreate(osThread(BLEmodule3Task), NULL); // Start BLEmodule task
}

static void BLE3Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule3Data;	//Structure to store the BLE module data
  const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
  unsigned int previousQueuing = 0;     //Store the time of the previous post in main queue
  BLEmodule3_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {

	if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
	{/* It is time to send a new message */
	  xSemaphoreTake(wakeBLEmodule3Task, portMAX_DELAY);      // Wait until something happens
	  xQueueReceive(BLEmodule3eventQueue, &sensorEvent, 0);
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule3Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule3Instrument)                              // Time to stream a message
	  {
		imu_100Hz_data_t * pData = NULL;
		pData 				     = (imu_100Hz_data_t*) pBLEmodule3Instrument->data;
		*pData 				     = resBLEmodule3Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLE3Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  previousQueuing = xTaskGetTickCount();
	}
  }
}

static void BLEmodule3_init(void)
{
  if (BLEmodule3_Config_Init())
  { // configuration is decoded and pointer handler is linked to BLE module 3
    wakeBLEmodule3Task = xSemaphoreCreateBinary(); // Creating binary semaphore
    BLEmodule3eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE3_DBG
    if (BLEmodule3eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLEmodule3_init] Error creating BLE module 3 event queue.\n", 0);
    }
#endif
  }
  else
  {
    xQueueSend(pPrintQueue, "[APP_BLEmodule3] [BLEmodule3_init] BLE module 3 event queue not created.\n", 0);
  }
}

void sensorHandlerBLEmodule3(imu_100Hz_data_t *sensorEvent)
{
  xQueueSend(BLEmodule3eventQueue, sensorEvent, 0);
  xSemaphoreGive(wakeBLEmodule3Task);
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
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule3Instrument, SETUP_PRM_COMM_METHOD_BT, 2);
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
