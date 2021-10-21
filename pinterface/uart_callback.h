/*
 * uart_callback.h
 *
 *  Created on: Sep 5, 2019
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 */

#ifndef UART_CALLBACK_H_
#define UART_CALLBACK_H_
#include "usart.h"
#include "pUART.h"

void uart3_callback(void);
void uart4_callback(void);
void uart5_callback(void);

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#endif /* UART_CALLBACK_H_ */
