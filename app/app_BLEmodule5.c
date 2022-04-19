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

void BLEmodule5_task_init()
{
  osThreadDef(BLEmodule5Task, BLE5Task, osPriorityNormal, 0, 1024); //Declaration of BLEmodule task
  BLEmodule5TaskHandle = osThreadCreate(osThread(BLEmodule5Task), NULL); // Start BLEmodule task
}

static void BLE5Task(const void * params)
{
  imu_100Hz_data_t sensorEvent;
  imu_100Hz_data_t resBLEmodule5Data;	//Structure to store the BLE module data
  const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
  unsigned int previousQueuing = 0;     //Store the time of the previous post in main queue
  BLEmodule5_init();                    //Initialize BLE module and RTOS resources.
  for(;;)
  {

	if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
	{/* It is time to send a new message */
	  xSemaphoreTake(wakeBLEmodule5Task, portMAX_DELAY);      // Wait until something happens
	  xQueueReceive(BLEmodule5eventQueue, &sensorEvent, 0);
	  refreshBLEmoduleData(&sensorEvent, &resBLEmodule5Data); // Copy actual BLE module data to structure that will be queued by reference
	  if (pBLEmodule5Instrument)                              // Time to stream a message
	  {
		imu_100Hz_data_t * pData = NULL;
		pData 				     = (imu_100Hz_data_t*) pBLEmodule5Instrument->data;
		*pData 				     = resBLEmodule5Data;
	  }
	  else
	  {
	    xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLE5Task] pBLEmoduleInstrument = 0.\n", 0);
	  }
	  previousQueuing = xTaskGetTickCount();
	}
  }
}

static void BLEmodule5_init(void)
{
  if (BLEmodule5_Config_Init())
  { // configuration is decoded and pointer handler is linked to BLE module 5
    wakeBLEmodule5Task = xSemaphoreCreateBinary(); // Creating binary semaphore
    BLEmodule5eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Create queue to store data from BLE instrument
#if PRINTF_APP_BLEMODULE5_DBG
    if (BLEmodule5eventQueue == NULL)
    {
	  xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLEmodule5_init] Error creating BLE module 5 event queue.\n", 0);
    }
#endif
  }
  else
  {
    xQueueSend(pPrintQueue, "[APP_BLEmodule5] [BLEmodule5_init] BLE module 5 event queue not created.\n", 0);
  }
}

void sensorHandlerBLEmodule5(imu_100Hz_data_t *sensorEvent)
{
  xQueueSend(BLEmodule5eventQueue, sensorEvent, 0);
  xSemaphoreGive(wakeBLEmodule5Task);
}

int BLEmodule5_Config_Init()
{
  CONFIG_WAITING_FOR_DECODE(); // Wait until a configuration is decoded and valid
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT); // Get number of BLE modules from config
  if (n >= 1)
  { // BLE modules are available, link the BLE module pointer handler to instrument_config_t structure from decodedConfig structure
//#if PRINTF_APP_BLEMODULE5_DBG
//    sprintf(string, "[APP_BLEmodule5] [BLEmodule5_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
//    xQueueSend(pPrintQueue, string, 0);
//#endif
	return getInstrumentFromConfig(&decodedConfig.conf, &pBLEmodule5Instrument, SETUP_PRM_COMM_METHOD_BT, 4);
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
