/*
 * timer_callback.c
 *
 *  Created on: Sep 6, 2019
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 */

#include "timer_callback.h"
#include "tim.h"
#include "../app/app_rtc.h"
#include "../app/app_sync.h"
#include "../Inc/imu_com.h"
#include "../Inc/sd_card_com.h"

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];
extern char string[];

#define PRINTF_DBG_timer 1

unsigned int tim2_start(void)
{
	// Start the timer here
	if (HAL_TIM_Base_Start_IT(&htim2) == HAL_OK)
	{
		return 1;
	}
	return 0;
}

void tim2_callback(void) // every 5ms
{
	static unsigned int localCounter = 0;

	if ((++localCounter) == 4) // every 20ms! -> if this is being changed, change also streamPeriod in app_imu.c
	//if ((++localCounter) == 2) // every 10ms!
	{
		localCounter = 0;
		app_rtc_inc_cycle_counter(); // Increment the cycle counter
		app_sync_notify_from_isr(1); // Notify the sync thread
	}

	//HAL_GPIO_TogglePin(LED_GOOD_GPIO_Port, LED_GOOD_Pin);
//  HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
//  IMU_sync_handler(&imu_1, *imu_array);
  // Todo HIER NOG IETS VOORZIEN, BAT SP UITMETEN EN WAARSCHUWEN
}

void ticker_1ms_callback()
{
	/* Called every 1ms */
	rtos_rtc_increment();
}
