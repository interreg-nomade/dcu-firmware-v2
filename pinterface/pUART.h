/*
 * uart.h
 *
 *  Created on: Nov 20, 2018
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 */

#ifndef INTERFACES_PUART_H_
#define INTERFACES_PUART_H_

#include "usart.h"
#include "stdbool.h"

#define UART1_INTERFACE_ENABLE		0
#define UART2_INTERFACE_ENABLE		0
#define UART3_INTERFACE_ENABLE		1
#define UART5_INTERFACE_ENABLE		0
#define UART6_INTERFACE_ENABLE		0
#define UART8_INTERFACE_ENABLE		0

//extern  uint8_t uart1rxbuf;

//void UART2_Init_Protection(void);
void UART3_Init_Protection(void);
void UART4_Init_Protection(void);
void UART5_Init_Protection(void);
//void UART8_Init_Protection(void);

/* Write several bytes */
//int UART2_WriteBytes(uint8_t* data, uint32_t n);
int UART3_WriteBytes(uint8_t* data, uint32_t n);
//int UART5_WriteBytes(uint8_t* data, uint32_t n);
//int UART8_WriteBytes(uint8_t* data, uint32_t n);

//void UART2_ReleaseSemFromISR(void);
void UART3_ReleaseSemFromISR(void);

//int UART1_Init_LinX();
//int UART1_Init_PG();
//int UART1_Init_GPSB();

//void UART1_Byte_Rx_Cb(char c);
//void UART_Tx_Cplt_Cb();

//int UART6_Init();

#endif /* INTERFACES_PUART_H_ */
