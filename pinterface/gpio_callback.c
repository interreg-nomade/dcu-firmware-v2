/*
 * gpio_callback.c
 *
 *  Created on: August, 31 2020
 *      Author: Sarah Goossens
 */
#include "gpio_callback.h"
#include "gpio.h"
#include "imu_com.h"

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

void HAL_GPIO_EXTI_Callback(uint16_t n)
{
	if(n == USER_BUTTON_Pin){ //GPIO_PIN_2
		//HAL_GPIO_TogglePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin);
		//IMU_go_to_sleep(&imu_1);
		//IMU_go_to_sleep(&imu_2);
		//IMU_go_to_sleep(&imu_3);
		//IMU_go_to_sleep(&imu_4);
		//IMU_go_to_sleep(&imu_5);
		//IMU_go_to_sleep(&imu_6);
	}
}
