/*
 * app_imuCalibration.c
 *
 *  Created on: 29 apr. 2022
 *      Author: Sarah Goossens
 */

#include <stdio.h>
#include <string.h>
#include "main.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "app_imuCalibration.h"
#include "common.h"
#include "../config/project_config.h"
#include "app_init.h"

#define PRINTF_APP_IMUCALIBRATION_DBG			1

SemaphoreHandle_t wakeIMUCalibrationTask;
static osThreadId IMUCalibrationTaskHandle;
uint8_t calibrateIMUs = 0;

extern char string[];
extern QueueHandle_t pPrintQueue;

extern int numberOfModules;
extern int numberOfModulesConnected;

void IMUCalibrationTask_init()
{
  osThreadDef(IMUCalibrationTaskRef, IMUCalibrationTask, osPriorityNormal, 0, 1024);  /* Declaration of IMUCalibrationTask */
  IMUCalibrationTaskHandle = osThreadCreate(osThread(IMUCalibrationTaskRef), NULL);   /* Start IMUCalibrationTask */
}

void IMUCalibrationTask(const void * params)
{
  wakeIMUCalibrationTask = xSemaphoreCreateBinary(); // Creating binary semaphore
  for(;;)
  {
	xSemaphoreTake(wakeIMUCalibrationTask, portMAX_DELAY);  // Wait until SHLD_BUTTON_2 is being pushed
	if (numberOfModulesConnected != numberOfModules)
	{
	  xQueueSend(pPrintQueue, "[app_imuCalibration] [IMUCalibrationTask] SHLD_BUTTON_2 pressed.\n", 0);
	  HAL_GPIO_WritePin(SHLD_BUTTON_2_LED_GPIO_Port, SHLD_BUTTON_2_LED_Pin, GPIO_PIN_SET);
	  calibrateIMUs = 1;
    }
	osDelay(20);
  }
}
