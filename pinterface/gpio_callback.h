/*
 * gpio_callback.h
 *
 *  Created on: August 31, 2020
 *      Author: Sarah Goossens
 */

#ifndef GPIO_CALLBACK_H_
#define GPIO_CALLBACK_H_

#include <stdint.h>

void HAL_GPIO_EXTI_Callback(uint16_t n);

#endif /* GPIO_CALLBACK_H_ */
