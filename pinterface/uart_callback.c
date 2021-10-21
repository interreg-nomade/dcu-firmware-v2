/*
 * uart_callback.c
 *
 *  Created on: Sep 5, 2019
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 */

#include "uart_callback.h"

#include <string.h>
#include "../lib/tablet_com_protocol/parser.h"
#include "app_tablet_com.h"
#include "../app/app_terminal_com.h"
#include "../app/app_BT1_com.h"
#include "app_init.h" // to declare QueueHandle_t

#define PRINTF_UART3_CALLBACK 0 // Tablet
#define PRINTF_UART4_CALLBACK 0 // BT1
#define PRINTF_UART5_CALLBACK 0 // Terminal

//#include "UartRingbufferManager.h"
//#include "UartRingbuffer.h"
//extern ring_buffer huart5_rx_buffer;

extern char string[];
extern QueueHandle_t pPrintQueue;

/* Handle reception of bytes from GPS: */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	/*   Prevent unused argument(s) compilation warning*/
	UNUSED(huart);
	if (huart->Instance == USART3)
	{
		UART3_ReleaseSemFromISR();
	}
//	else if (huart->Instance == USART1)
//	{
//		UART_Tx_Cplt_Cb();
//	}
//
//	/*if (huart->Instance == UART7)
//	{
//		//pg_tx_int_callback();
//	} */
}

void uart3_callback(void)
{

	if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_RXNE))
	{
#if PRINTF_UART3_CALLBACK
		sprintf(string, "GPIO uart3_callback (Android USB).\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		/*Clear the interrupt by reading the DR:*/
		char rxdByte;
		rxdByte = huart3.Instance->RDR;
		//printf("rx:%d\n", rxdByte);
		cpl_RxHandler(rxdByte);
		if(cpl_RxBufferSize() >= 1)
		{
			/* Notify the uplink Manager thread from ISR */
			app_tablet_com_notify_from_isr(0x01);
		}
	}

}

void uart4_callback(void)
{
#if PRINTF_UART4_CALLBACK
	sprintf(string, "[uart_callback] [uart4_callback]\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	if (__HAL_UART_GET_FLAG(&huart4, UART_FLAG_RXNE))
	{
		/*Clear the interrupt by reading the DR:*/
		char rxdByte;
		rxdByte = huart4.Instance->RDR;
#if PRINTF_UART4_CALLBACK
		sprintf(string, "[uart_callback] [uart4_callback] Received byte = 0x%02X.\n",rxdByte);
        xQueueSend(pPrintQueue, string, 0);
#endif
		bt_RxHandler(rxdByte);
	}
}

void uart5_callback(void)
{
#if PRINTF_UART5_CALLBACK
	sprintf(string, "[uart_callback] [uart5_callback]\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	if (__HAL_UART_GET_FLAG(&huart5, UART_FLAG_RXNE))
	{
		/*Clear the interrupt by reading the DR:*/
		char rxdByte;
		rxdByte = huart5.Instance->RDR;
#if PRINTF_UART5_CALLBACK
		sprintf(string, "[uart_callback] [uart5_callback] Received byte = 0x%02X.\n",rxdByte);
        xQueueSend(pPrintQueue, string, 0);
#endif
		com_RxHandler(rxdByte);
	}
}
