/*
 * uart.c
 *
 *  Created on: Nov 20, 2018
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 */

#include "pUART.h"
#include "FreeRTOS.h"
#include "semphr.h"
#include "../config/project_config.h"





#include "pUART_Callbacks.h"
#include "string.h"

#define UART_USING_IT
#define pUART_DBG_PRINTF 0
#define PRINTF_PUART_UPLINK_DBG 1

extern char string[];
extern QueueHandle_t pPrintQueue;

#if UART1_INTERFACE_ENABLE
void(*UART1_Rx_pCb)(uint8_t c);
void(*UART1_Tx_CpltCb)();

void UART1_Init(void)
{
	huart1.Instance = USART1;
#ifdef PWC_INTERFACE_LINX
	huart1.Init.BaudRate = 115200;
#else
	huart1.Init.BaudRate = 38400;
#endif
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.Prescaler = UART_PRESCALER_DIV1;
	huart1.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	huart1.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	huart1.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
	huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
	}
	//__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); //TODO place it properly
	//SET_BIT(huart1.Instance->CR3, 0x01);
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1rxbuf, 1);
}

int UART1_Init_GPSB()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 38400;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.Prescaler = UART_PRESCALER_DIV1;
	huart1.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	huart1.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	huart1.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
	huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
		return 0;
	}
	UART1_Rx_pCb = GPSB_RxHandler;
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1rxbuf, 1);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); //TODO place it properly
	SET_BIT(huart1.Instance->CR3, 0x01);
	return 1;
}

int UART1_Init_PG()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 38400;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.Prescaler = UART_PRESCALER_DIV1;
	huart1.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	huart1.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	huart1.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
	huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
		return 0;
	}
	UART1_Rx_pCb    = pg_rx_int_callback;
	UART1_Tx_CpltCb = png_txcplt_callback;

	HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1rxbuf, 1);
	__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); //TODO place it properly
	SET_BIT(huart1.Instance->CR3, 0x01);
	return 1;
}

int UART1_Init_LinX()
{
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 115200;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.Init.Prescaler = UART_PRESCALER_DIV1;
	huart1.Init.FIFOMode = UART_FIFOMODE_DISABLE;
	huart1.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
	huart1.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_RXOVERRUNDISABLE_INIT;
	huart1.AdvancedInit.OverrunDisable = UART_ADVFEATURE_OVERRUN_DISABLE;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		//_Error_Handler(__FILE__, __LINE__);
		return 0;
	}
	UART1_Rx_pCb    = linxReadCharImplt;
	//__HAL_UART_ENABLE_IT(&huart1, UART_IT_RXNE); //TODO place it properly
	SET_BIT(huart1.Instance->CR3, 0x01);
	HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart1rxbuf, 1);
	return 1;
}

void UART1_Byte_Rx_Cb(char c)
{
	if (UART1_Rx_pCb)
	{
		UART1_Rx_pCb(c);
	}
}

void UART_Tx_Cplt_Cb()
{
	if (UART1_Tx_CpltCb)
	{
		UART1_Tx_CpltCb();
	}
}
#endif

#if UART2_INTERFACE_ENABLE
static SemaphoreHandle_t xUART2_TxSemaphore  = NULL;
static SemaphoreHandle_t xUART2_TxMutex   	 = NULL;

unsigned int UART2_TakeSem();
unsigned int UART2_GiveSem();

unsigned int UART2_TakeMutex();
unsigned int UART2_GiveMutex();

void UART2_Init_Protection(void)
{

	/* Initialize the UART TX Semaphore */
	xUART2_TxSemaphore = xSemaphoreCreateBinary();
	if(xUART2_TxSemaphore != NULL)
	{
		/* The semaphore can be used.
		 * Note : The semaphore must be given first
		 * or calling xSemaphoreTake() would failed */
	}
	/* Initialize the UART TX Mutex     */
	xUART2_TxMutex = xSemaphoreCreateMutex();
	if(xUART2_TxMutex != NULL)
	{
		/* The mutex as been create successfully
		 * It can be used by using xSemaphoreTake()
		 * function
		 */
	}
	MX_USART2_UART_Init();
	HAL_NVIC_SetPriority(USART2_IRQn, 7, 0);
	HAL_NVIC_EnableIRQ(USART2_IRQn);
	/* Enable RX Not Empty interrupt */
	__HAL_UART_ENABLE_IT(&huart2, UART_IT_RXNE);
	/* Enable the error interrupt (it is handled by the ST library in ISR context) */
	SET_BIT(huart2.Instance->CR3, 0x01);

}

unsigned int UART2_TakeMutex()
{
	if(xSemaphoreTake(xUART2_TxMutex, (TickType_t) 400))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int UART2_GiveMutex()
{
	if(xSemaphoreGive(xUART2_TxMutex))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int UART2_TakeSem()
{
	if (xSemaphoreTake(xUART2_TxSemaphore, ( TickType_t ) 400)) {
		return 1;
	} else {
		return 0;
	}
}
unsigned int UART2_GiveSem()
{
	if (xSemaphoreGive(xUART2_TxSemaphore)) {
		return 1;
	} else {
		return 0;
	}
}
void UART2_ReleaseSemFromISR(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Unblock the task by releasing the semaphore. */
	xSemaphoreGiveFromISR(xUART2_TxSemaphore, &xHigherPriorityTaskWoken);
	/* If xHigherPriorityTaskWoken was set to true you
	 we should yield.  The actual macro used here is
	 port specific. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

int UART2_WriteBytes(uint8_t * data, uint32_t n)
{
#ifdef UART_USING_IT
	HAL_StatusTypeDef res;

	/* As the ressource is shared between the Streamer and tablet com task we need
	 * to lock with a Mutex
	 */
	if(UART2_TakeMutex)
	{
		res = HAL_UART_Transmit_IT(&huart2, (uint8_t *)data, n);

		if(res == HAL_OK)
		{
			/* Uart operation succeed */
			/* Block on the semaphore */
			/* The semaphore is given in the UART interruption at the end
			 * of the transmission */
			if(UART2_TakeSem())
			{
				/* Take the semaphore succeed */
				/* Give the mutex  */
				UART2_GiveMutex();
				return 1;
			}
		}
		/* Release the mutex */
		UART2_GiveMutex();
	}
	return 0;
#else
#endif
}
#endif

#if UART3_INTERFACE_ENABLE
static SemaphoreHandle_t xUART3_TxSemaphore  = NULL;
static SemaphoreHandle_t xUART3_TxMutex   	 = NULL;

/* Private functions */
unsigned int UART3_TakeSem();
unsigned int UART3_GiveSem();

unsigned int UART3_TakeMutex();
unsigned int UART3_GiveMutex();

void UART3_Init_Protection(void)
{
	/* Initialize the UART TX Semaphore */
	xUART3_TxSemaphore = xSemaphoreCreateBinary();
	if(xUART3_TxSemaphore != NULL)
	{
		/* The semaphore can be used.
		 * Note : The semaphore must be given first
		 * or calling xSemaphoreTake() would failed */
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_Init_Protection] xUART3_TxSemaphore created: 0x%0X.\n", (unsigned int) xUART3_TxSemaphore);
      xQueueSend(pPrintQueue, string, 0);
#endif
	}
	else
	{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_Init_Protection] xUART3_TxSemaphore not created: 0x%0X.\n", (unsigned int) xUART3_TxSemaphore);
      xQueueSend(pPrintQueue, string, 0);
#endif
	}
	/* Initialize the UART TX Mutex     */
	xUART3_TxMutex = xSemaphoreCreateMutex();
	if(xUART3_TxMutex != NULL)
	{
		/* The mutex as been create successfully
		 * It can be used by using xSemaphoreTake()
		 * function
		 */
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_Init_Protection] xUART3_TxMutex created: 0x%0X.\n", (unsigned int) xUART3_TxMutex);
      xQueueSend(pPrintQueue, string, 0);
#endif
	}
	else
	{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_Init_Protection] xUART3_TxMutex not created: 0x%0X.\n", (unsigned int) xUART3_TxMutex);
      xQueueSend(pPrintQueue, string, 0);
#endif
	}
	MX_USART3_UART_Init();
	HAL_NVIC_SetPriority(USART3_IRQn, 8, 0);
	HAL_NVIC_EnableIRQ(USART3_IRQn);
	/* Enable RX Not Empty interrupt */
	__HAL_UART_ENABLE_IT(&huart3, UART_IT_RXNE);
	/* Enable the error interrupt (it is handled by the ST library in ISR context) */
	SET_BIT(huart3.Instance->CR3, 0x01);
}

unsigned int UART3_TakeMutex()
{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_TakeMutex] started: 0x%0X.\n", (unsigned int) xUART3_TxMutex);
      xQueueSend(pPrintQueue, string, 0);
#endif
	if(xSemaphoreTake(xUART3_TxMutex, (TickType_t) 400))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int UART3_GiveMutex()
{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_GiveMutex] started: 0x%0X.\n", (unsigned int) xUART3_TxMutex);
      xQueueSend(pPrintQueue, string, 0);
#endif
	if(xSemaphoreGive(xUART3_TxMutex))
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

unsigned int UART3_TakeSem()
{
	if (xSemaphoreTake(xUART3_TxSemaphore, ( TickType_t ) 400)) {
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_TakeSem] Semaphore taken.\n");
      xQueueSend(pPrintQueue, string, 0);
#endif
	  return 1;
	}
	else
	{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_TakeSem] Semaphore time out.\n");
      xQueueSend(pPrintQueue, string, 0);
#endif
	  return 0;
	}
}

unsigned int UART3_GiveSem()
{
#if pUART_DBG_PRINTF
	  sprintf(string, "[pUART] [UART3_GiveSem] started: 0x%0X.\n", (unsigned int) xUART3_TxSemaphore);
      xQueueSend(pPrintQueue, string, 0);
#endif
	if (xSemaphoreGive(xUART3_TxSemaphore)) {
		return 1;
	} else {
		return 0;
	}
}

void UART3_ReleaseSemFromISR(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	/* Unblock the task by releasing the semaphore. */
	xSemaphoreGiveFromISR(xUART3_TxSemaphore, &xHigherPriorityTaskWoken);
	/* If xHigherPriorityTaskWoken was set to true you
	 we should yield.  The actual macro used here is
	 port specific. */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/* Write several bytes */
int UART3_WriteBytes(uint8_t* data, uint32_t n)
{

#ifdef UART_USING_IT
	HAL_StatusTypeDef res;

	/* As the resource is shared between the Streamer and tablet com task we need to lock with a Mutex */
	if(UART3_TakeMutex()) //! hier stond enkel UART3_TakeMutex, wat altijd TRUE is!
	{
		res = HAL_UART_Transmit_IT(&huart3, (uint8_t*)data, n);
		if (res == HAL_OK)
		{
			/* Uart operation succeed */
			/* Block on the semaphore */
			/* The semaphore is given in the UART interruption at the end of the transmission */
			if (UART3_TakeSem())
			{
				/* Take the semaphore succeed */
				/* Give the mutex  */
				UART3_GiveMutex();
				return 1;
			}
		}
		/* Release the mutex */
		UART3_GiveMutex();
	}
	return 0;
#endif
}
#endif

void UART4_Init_Protection(void)
{
	/* Initialize the UART TX Semaphore */
//	xUART5_TxSemaphore = xSemaphoreCreateBinary();
	/* Initialize the UART TX Mutex     */
//	xUART5_TxMutex = xSemaphoreCreateMutex(); FTDI
	MX_UART4_Init(BT_BAUDRATE);
	HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(UART4_IRQn);
	/* Enable RX Not Empty interrupt */
	__HAL_UART_ENABLE_IT(&huart4, UART_IT_RXNE);
	/* Enable the error interrupt (it is handled by the ST library in ISR context) */
	SET_BIT(huart4.Instance->CR3, 0x01);
}

void UART5_Init_Protection(void)
{
	/* Initialize the UART TX Semaphore */
//	xUART5_TxSemaphore = xSemaphoreCreateBinary();
	/* Initialize the UART TX Mutex     */
//	xUART5_TxMutex = xSemaphoreCreateMutex(); FTDI
	MX_UART5_Init(COM_BAUDRATE);
	HAL_NVIC_SetPriority(UART5_IRQn, 9, 0);
	HAL_NVIC_EnableIRQ(UART5_IRQn);
	/* Enable RX Not Empty interrupt */
	__HAL_UART_ENABLE_IT(&huart5, UART_IT_RXNE);
	/* Enable the error interrupt (it is handled by the ST library in ISR context) */
	SET_BIT(huart5.Instance->CR3, 0x01);
}
