/**
  ******************************************************************************
  * File Name          : USART.c
  * Description        : This file provides code for the configuration
  *                      of the USART instances.
  ******************************************************************************
  * This notice applies to any and all portions of this file
  * that are not between comment pairs USER CODE BEGIN and
  * USER CODE END. Other portions of this file, whether 
  * inserted by the user or by software development tools
  * are owned by their respective copyright owners.
  *
  * Copyright (c) 2019 STMicroelectronics International N.V. 
  * All rights reserved.
  *
  * Redistribution and use in source and binary forms, with or without 
  * modification, are permitted, provided that the following conditions are met:
  *
  * 1. Redistribution of source code must retain the above copyright notice, 
  *    this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  *    this list of conditions and the following disclaimer in the documentation
  *    and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of other 
  *    contributors to this software may be used to endorse or promote products 
  *    derived from this software without specific written permission.
  * 4. This software, including modifications and/or derivative works of this 
  *    software, must execute solely and exclusively on microcontroller or
  *    microprocessor devices manufactured by or for STMicroelectronics.
  * 5. Redistribution and use of this software other than as permitted under 
  *    this license is void and will automatically terminate your rights under 
  *    this license. 
  *
  * THIS SOFTWARE IS PROVIDED BY STMICROELECTRONICS AND CONTRIBUTORS "AS IS" 
  * AND ANY EXPRESS, IMPLIED OR STATUTORY WARRANTIES, INCLUDING, BUT NOT 
  * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
  * PARTICULAR PURPOSE AND NON-INFRINGEMENT OF THIRD PARTY INTELLECTUAL PROPERTY
  * RIGHTS ARE DISCLAIMED TO THE FULLEST EXTENT PERMITTED BY LAW. IN NO EVENT 
  * SHALL STMICROELECTRONICS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
  * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, 
  * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF 
  * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING 
  * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "usart.h"

/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

UART_HandleTypeDef huart4;  // BT1    --> changed in V2.0 to BLE and on other pins
UART_HandleTypeDef huart5;  // CH340  --> moved in V2.0 to huart7
UART_HandleTypeDef huart7;  // BT3    --> not in use in V2.0
UART_HandleTypeDef huart8;  // BT4    --> not in use in V2.0
UART_HandleTypeDef huart1;  // BT5    --> not in use in V2.0
UART_HandleTypeDef huart2;  // BT6    --> not in use in V2.0
UART_HandleTypeDef huart3;  // TABLET --> remains the same in V2.0
UART_HandleTypeDef huart6;  // BT2    --> not in use in V2.0

void MX_UART4_Init(uint32_t baudrate)
{
  huart4.Instance = UART4;
  huart4.Init.BaudRate =   1000000; // baudrate;
  huart4.Init.WordLength = UART_WORDLENGTH_8B;
  huart4.Init.StopBits = UART_STOPBITS_1;
  huart4.Init.Parity = UART_PARITY_NONE;
  huart4.Init.Mode = UART_MODE_TX_RX;
  huart4.Init.HwFlowCtl =  UART_HWCONTROL_NONE; //UART_HWCONTROL_RTS_CTS;
  huart4.Init.OverSampling = UART_OVERSAMPLING_16;
  huart4.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart4.Init.Prescaler = UART_PRESCALER_DIV1;
  huart4.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart4.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart4.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart4.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart4) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_UART5_Init(uint32_t baudrate)
{
  huart5.Instance = UART5;
  huart5.Init.BaudRate = baudrate;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.Init.Prescaler = UART_PRESCALER_DIV1;
  huart5.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart5.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart5.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_UART7_Init(uint32_t baudrate)
{
  huart7.Instance = UART7;
  huart7.Init.BaudRate = baudrate;
  huart7.Init.WordLength = UART_WORDLENGTH_8B;
  huart7.Init.StopBits = UART_STOPBITS_1;
  huart7.Init.Parity = UART_PARITY_NONE;
  huart7.Init.Mode = UART_MODE_TX_RX;
  huart7.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS; //UART_HWCONTROL_NONE;
  huart7.Init.OverSampling = UART_OVERSAMPLING_16;
  huart7.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart7.Init.Prescaler = UART_PRESCALER_DIV1;
  huart7.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart7.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart7.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart7.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart7) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_UART8_Init(uint32_t baudrate)
{
  huart8.Instance = UART8;
  huart8.Init.BaudRate = baudrate;
  huart8.Init.WordLength = UART_WORDLENGTH_8B;
  huart8.Init.StopBits = UART_STOPBITS_1;
  huart8.Init.Parity = UART_PARITY_NONE;
  huart8.Init.Mode = UART_MODE_TX_RX;
  huart8.Init.HwFlowCtl = UART_HWCONTROL_NONE; //UART_HWCONTROL_RTS_CTS;
  huart8.Init.OverSampling = UART_OVERSAMPLING_16;
  huart8.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart8.Init.Prescaler = UART_PRESCALER_DIV1;
  huart8.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart8.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart8.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart8.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart8) != HAL_OK)
  {
    Error_Handler();
  }
}


void MX_USART1_UART_Init(uint32_t baudrate)
{
  huart1.Instance = USART1;
  huart1.Init.BaudRate = baudrate;
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
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_USART2_UART_Init(uint32_t baudrate)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = baudrate;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE; //UART_HWCONTROL_RTS_CTS;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.Init.Prescaler = UART_PRESCALER_DIV1;
  huart2.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart2.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart2.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

//void MX_USART3_UART_Init(uint32_t baudrate)
void MX_USART3_UART_Init(void)
{
  huart3.Instance = USART3;
//  huart3.Init.BaudRate = baudrate;
  huart3.Init.BaudRate = 921600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_RTS_CTS; // UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart3.Init.Prescaler = UART_PRESCALER_DIV1;
  huart3.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart3.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart3.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
}

void MX_USART6_UART_Init(uint32_t baudrate)
{
  huart6.Instance = USART6;
  huart6.Init.BaudRate = baudrate;
  huart6.Init.WordLength = UART_WORDLENGTH_8B;
  huart6.Init.StopBits = UART_STOPBITS_1;
  huart6.Init.Parity = UART_PARITY_NONE;
  huart6.Init.Mode = UART_MODE_TX_RX;
  huart6.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart6.Init.OverSampling = UART_OVERSAMPLING_16;
  huart6.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart6.Init.Prescaler = UART_PRESCALER_DIV1;
  huart6.Init.FIFOMode = UART_FIFOMODE_DISABLE;
  huart6.Init.TXFIFOThreshold = UART_TXFIFO_THRESHOLD_1_8;
  huart6.Init.RXFIFOThreshold = UART_RXFIFO_THRESHOLD_1_8;
  huart6.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart6) != HAL_OK)
  {
    Error_Handler();
  }
}

void HAL_UART_MspInit(UART_HandleTypeDef* uartHandle)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
//  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  if(uartHandle->Instance==UART4) // BLE (nRF52)
  {
  /* USER CODE BEGIN UART4_MspInit 0 */

  /* USER CODE END UART4_MspInit 0 */
    /** Initializes the peripherals clock */
//    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_UART4;
//    PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_D2PCLK1;
//    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
//    {
//      Error_Handler();
//    }
    /* UART4 clock enable */
    __HAL_RCC_UART4_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();

    /**UART4 GPIO Configuration
    PA0      ------> UART4_TX
    PA1      ------> UART4_RX
    PB14     ------> UART4_RTS
    PB15     ------> UART4_CTS
    */

    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH; //GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF8_UART4;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* UART4 interrupt Init */
    //HAL_NVIC_SetPriority(UART4_IRQn, 0, 0);
    //HAL_NVIC_EnableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspInit 1 */

  /* USER CODE END UART4_MspInit 1 */
  }
  else if(uartHandle->Instance==UART5) // CH340 --> V2.0: not used (CH340 function moved to UART7)
  {
//  /* USER CODE BEGIN UART5_MspInit 0 */
//
//  /* USER CODE END UART5_MspInit 0 */
//    /* UART5 clock enable */
//    __HAL_RCC_UART5_CLK_ENABLE();
//
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//    /**UART5 GPIO Configuration
//    PB12     ------> UART5_RX
//    PB13     ------> UART5_TX
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF14_UART5;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//    /* UART5 interrupt Init */
////    HAL_NVIC_SetPriority(UART5_IRQn, 6, 0);
////    HAL_NVIC_EnableIRQ(UART5_IRQn);
//  /* USER CODE BEGIN UART5_MspInit 1 */
//
//  /* USER CODE END UART5_MspInit 1 */
  }
  else if(uartHandle->Instance==UART7) // BT3 --> V2.0: CH340
  {
  /* USER CODE BEGIN UART7_MspInit 0 */

  /* USER CODE END UART7_MspInit 0 */
    /* UART7 clock enable */
    __HAL_RCC_UART7_CLK_ENABLE();
  
    __HAL_RCC_GPIOF_CLK_ENABLE();
    /**UART7 GPIO Configuration    
    PF6     ------> UART7_RX
    PF7     ------> UART7_TX
    PF8     ------> UART7_RTS
    PF9     ------> UART7_CTS 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF7_UART7;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* UART7 interrupt Init */
    HAL_NVIC_SetPriority(UART7_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspInit 1 */

  /* USER CODE END UART7_MspInit 1 */
  }
  else if(uartHandle->Instance==UART8) // BT4 --> V2.0: not used
  {
//  /* USER CODE BEGIN UART8_MspInit 0 */
//
//  /* USER CODE END UART8_MspInit 0 */
//    /* UART8 clock enable */
//    __HAL_RCC_UART8_CLK_ENABLE();
//
//    __HAL_RCC_GPIOD_CLK_ENABLE();
//    __HAL_RCC_GPIOE_CLK_ENABLE();
//    /**UART8 GPIO Configuration
//    PD14     ------> UART8_CTS
//    PD15     ------> UART8_RTS
//    PE0     ------> UART8_RX
//    PE1     ------> UART8_TX
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_14|GPIO_PIN_15;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
//    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
//
//    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF8_UART8;
//    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);
//
//    /* UART8 interrupt Init */
//    HAL_NVIC_SetPriority(UART8_IRQn, 6, 0);
//    HAL_NVIC_EnableIRQ(UART8_IRQn);
//  /* USER CODE BEGIN UART8_MspInit 1 */
//
//  /* USER CODE END UART8_MspInit 1 */
  }
  else if(uartHandle->Instance==USART1) // BT5 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART1_MspInit 0 */
//
//  /* USER CODE END USART1_MspInit 0 */
//    /* USART1 clock enable */
//    __HAL_RCC_USART1_CLK_ENABLE();
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    __HAL_RCC_GPIOB_CLK_ENABLE();
//
//    /**USART1 GPIO Configuration
//    PA10     ------> USART1_RX
//    PA11     ------> USART1_CTS
//    PA12     ------> USART1_RTS
//    PB6      ------> USART1_TX
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//    GPIO_InitStruct.Pin = GPIO_PIN_6;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
//    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
//
//
//    /* USART1 interrupt Init */
//    HAL_NVIC_SetPriority(USART1_IRQn, 6, 0);
//    HAL_NVIC_EnableIRQ(USART1_IRQn);
//  /* USER CODE BEGIN USART1_MspInit 1 */
//
//  /* USER CODE END USART1_MspInit 1 */
  }
  else if(uartHandle->Instance==USART2) // BT6 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART2_MspInit 0 */
//
//  /* USER CODE END USART2_MspInit 0 */
//    /* USART2 clock enable */
//    __HAL_RCC_USART2_CLK_ENABLE();
//
//    __HAL_RCC_GPIOA_CLK_ENABLE();
//    /**USART2 GPIO Configuration
//    PA0     ------> USART2_CTS
//    PA1     ------> USART2_RTS
//    PA2     ------> USART2_TX
//    PA3     ------> USART2_RX
//    GPIO_InitStruct.Pin = GPIO_PIN_0;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//    */
//
//    GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
//    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
//
//    /* USART2 interrupt Init */
//    HAL_NVIC_SetPriority(USART2_IRQn, 6, 0);
//    HAL_NVIC_EnableIRQ(USART2_IRQn);
//  /* USER CODE BEGIN USART2_MspInit 1 */
//
//  /* USER CODE END USART2_MspInit 1 */
  }
  else if(uartHandle->Instance==USART3) // Tablet --> remains the same
  {
  /* USER CODE BEGIN USART3_MspInit 0 */

  /* USER CODE END USART3_MspInit 0 */
    /* USART3 clock enable */
    __HAL_RCC_USART3_CLK_ENABLE();
  
    __HAL_RCC_GPIOD_CLK_ENABLE();
    /**USART3 GPIO Configuration    
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    PD11     ------> USART3_CTS
    PD12     ------> USART3_RTS 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USART3 interrupt Init */
//    HAL_NVIC_SetPriority(USART3_IRQn, 8, 0);
//    HAL_NVIC_EnableIRQ(USART3_IRQn);

  /* USER CODE BEGIN USART3_MspInit 1 */

  /* USER CODE END USART3_MspInit 1 */
  }
  else if(uartHandle->Instance==USART6) // BT2 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART6_MspInit 0 */
//
//  /* USER CODE END USART6_MspInit 0 */
//    /* USART6 clock enable */
//    __HAL_RCC_USART6_CLK_ENABLE();
//
//    __HAL_RCC_GPIOG_CLK_ENABLE();
//    __HAL_RCC_GPIOC_CLK_ENABLE();
//    /**USART6 GPIO Configuration
//    PC6     ------> USART6_TX
//    PC7     ------> USART6_RX
//    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//    */
//
//    /**USART6 GPIO Configuration
//    PG8     ------> USART6_RTS
//    PC6     ------> USART6_TX
//    PC7     ------> USART6_RX
//    PG13     ------> USART6_CTS
//    */
//    GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_13;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
//    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);
//
//    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
//    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
//    GPIO_InitStruct.Pull = GPIO_NOPULL;
//    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
//    GPIO_InitStruct.Alternate = GPIO_AF7_USART6;
//    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
//
//    /* USART6 interrupt Init */
//    HAL_NVIC_SetPriority(USART6_IRQn, 6, 0);
//    HAL_NVIC_EnableIRQ(USART6_IRQn);
//  /* USER CODE BEGIN USART6_MspInit 1 */
//
//  /* USER CODE END USART6_MspInit 1 */
  }
}

void HAL_UART_MspDeInit(UART_HandleTypeDef* uartHandle)
{

  if(uartHandle->Instance==UART4) // BT1 --> V2.0: changed to BLE
  {
  /* USER CODE BEGIN UART4_MspDeInit 0 */

  /* USER CODE END UART4_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART4_CLK_DISABLE();
  
    /**UART4 GPIO Configuration
    PB15        ------> UART4_CTS  --> V2.0: remains the same
    PA15 (JTDI) ------> UART4_RTS  --> V2.0: becomes PB14
    PD0         ------> UART4_RX   --> V2.0: becomes PA1
    PD1         ------> UART4_TX   --> V2.0: becomes PA0

    */

//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_15);
//    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_0|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_15);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_14);
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1);

    /* UART4 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART4_IRQn);
  /* USER CODE BEGIN UART4_MspDeInit 1 */

  /* USER CODE END UART4_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART5)  // CH340 --> V2.0: not used (CH340 function moved to UART7)
  {
//  /* USER CODE BEGIN UART5_MspDeInit 0 */
//
//  /* USER CODE END UART5_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_UART5_CLK_DISABLE();
//
//    /**UART5 GPIO Configuration
//    PB12     ------> UART5_RX
//    PB13     ------> UART5_TX
//    */
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12|GPIO_PIN_13);
//
//    /* UART5 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(UART5_IRQn);
//  /* USER CODE BEGIN UART5_MspDeInit 1 */
//
//  /* USER CODE END UART5_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART7)  // BT3 --> V2.0: CH340
  {
  /* USER CODE BEGIN UART7_MspDeInit 0 */

  /* USER CODE END UART7_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_UART7_CLK_DISABLE();
  
    /**UART7 GPIO Configuration    
    PF6     ------> UART7_RX
    PF7     ------> UART7_TX
    PF8     ------> UART7_RTS
    PF9     ------> UART7_CTS 
    */
    HAL_GPIO_DeInit(GPIOF, GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9);

    /* UART7 interrupt Deinit */
    HAL_NVIC_DisableIRQ(UART7_IRQn);
  /* USER CODE BEGIN UART7_MspDeInit 1 */

  /* USER CODE END UART7_MspDeInit 1 */
  }
  else if(uartHandle->Instance==UART8) // BT4 --> V2.0: not used
  {
//  /* USER CODE BEGIN UART8_MspDeInit 0 */
//
//  /* USER CODE END UART8_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_UART8_CLK_DISABLE();
//
//    /**UART8 GPIO Configuration
//    PD14     ------> UART8_CTS
//    PD15     ------> UART8_RTS
//    PE0     ------> UART8_RX
//    PE1     ------> UART8_TX
//    */
//    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_14|GPIO_PIN_15);
//
//    HAL_GPIO_DeInit(GPIOE, GPIO_PIN_0|GPIO_PIN_1);
//
//    /* UART8 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(UART8_IRQn);
//  /* USER CODE BEGIN UART8_MspDeInit 1 */
//
//  /* USER CODE END UART8_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART1) // BT5 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART1_MspDeInit 0 */
//
//  /* USER CODE END USART1_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART1_CLK_DISABLE();
//
//    /**USART1 GPIO Configuration
//    PA10     ------> USART1_RX
//    PA11     ------> USART1_CTS
//    PA12     ------> USART1_RTS
//    PB6     ------> USART1_TX
//    */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12);
//    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6);
//
//    /* USART1 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(USART1_IRQn);
//  /* USER CODE BEGIN USART1_MspDeInit 1 */
//
//  /* USER CODE END USART1_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART2) // BT6 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART2_MspDeInit 0 */
//
//  /* USER CODE END USART2_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART2_CLK_DISABLE();
//
//    /**USART2 GPIO Configuration
//    PA0     ------> USART2_CTS
//    PA1     ------> USART2_RTS
//    PA2     ------> USART2_TX
//    PA3     ------> USART2_RX
//    */
//    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3);
//
//    /* USART2 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(USART2_IRQn);
//  /* USER CODE BEGIN USART2_MspDeInit 1 */
//
//  /* USER CODE END USART2_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART3) // Tablet --> remains the same
  {
  /* USER CODE BEGIN USART3_MspDeInit 0 */

  /* USER CODE END USART3_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_USART3_CLK_DISABLE();
  
    /**USART3 GPIO Configuration
    PD8     ------> USART3_TX
    PD9     ------> USART3_RX
    PD11     ------> USART3_CTS
    PD12     ------> USART3_RTS 
    */
    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_11|GPIO_PIN_12);

    /* USART2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(USART3_IRQn);

  /* USER CODE BEGIN USART3_MspDeInit 1 */

  /* USER CODE END USART3_MspDeInit 1 */
  }
  else if(uartHandle->Instance==USART6) // BT2 --> V2.0: not used
  {
//  /* USER CODE BEGIN USART6_MspDeInit 0 */
//
//  /* USER CODE END USART6_MspDeInit 0 */
//    /* Peripheral clock disable */
//    __HAL_RCC_USART6_CLK_DISABLE();
//
//    /**USART6 GPIO Configuration
//    PG8     ------> USART6_RTS
//    PC6     ------> USART6_TX
//    PC7     ------> USART6_RX
//    PG13     ------> USART6_CTS
//    */
//    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_8|GPIO_PIN_13);
//    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_6|GPIO_PIN_7);
//
//    /* USART6 interrupt Deinit */
//    HAL_NVIC_DisableIRQ(USART6_IRQn);
//  /* USER CODE BEGIN USART6_MspDeInit 1 */
//
//  /* USER CODE END USART6_MspDeInit 1 */
  }
} 

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
