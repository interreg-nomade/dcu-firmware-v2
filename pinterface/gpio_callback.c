/*
 * gpio_callback.c
 *
 *  Created on: August, 31 2020
 *      Author: Sarah Goossens
 */
#include "gpio_callback.h"
#include "gpio.h"
#include "app_imuCalibration.h"
#include "FreeRTOS.h"
#include "semphr.h"

extern SemaphoreHandle_t wakeIMUCalibrationTask;

BaseType_t xHigherPriorityTaskWoken = pdFALSE;

void HAL_GPIO_EXTI_Callback(uint16_t n)
{
  if (n == SHLD_BUTTON_2_Pin)
  {
	xSemaphoreGiveFromISR(wakeIMUCalibrationTask, &xHigherPriorityTaskWoken);
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }
  else
  {
    if (n == USER_BUTTON_Pin)
    {
      HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    }
  }
}
