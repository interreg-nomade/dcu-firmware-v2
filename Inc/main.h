/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32h7xx_hal.h"
#include "stm32h7xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

#define DCU_SW_VERSION  SW_V31

#define SW_V30 	30
#define SW_V31 	31

#define TICKPRIORITY     				 		1

#define BT_BAUDRATE								115200
#define COM_BAUDRATE							2000000 //921600
#define SIZE_TERMINAL_PACKET 					1
#define TABLET_BAUDRATE							921600 //921600
#define SIZE_TABLET_PACKET						11

#define NUMBER_OF_BT_PACKETS					1 // 2

#define PACKET_START_POS						11
#define NUMBER_OF_DATA_READS_IN_BT_PACKET		10
#define SIZE_SAMPLE_FRAME						8
#define SIZE_CHECKSUM							1
#define SIZE_BT_PACKET							92 //PACKET_START_POS + NUMBER_OF_DATA_READS_IN_BT_PACKET * SIZE_SAMPLE_FRAME + SIZE_CHECKSUM  //92
#define SIZE_PING_PONG_BUFFER					SIZE_BT_PACKET * NUMBER_OF_BT_PACKETS
#define SIZE_SD_CARD_READ_BUF					NUMBER_OF_BT_PACKETS * NUMBER_OF_DATA_READS_IN_BT_PACKET
#define NUMBER_OF_SENSOR_SLOTS					1


#define I2C_ADDRESS_IC5_DATASHEET 	0x38
#define I2C_ADDRESS_IC6_DATASHEET   0x39
#define I2C_ADDRESS_IC7_DATASHEET   0x40

#define I2C_ADDRESS_IC5 					I2C_ADDRESS_IC5_DATASHEET << 1
#define I2C_ADDRESS_IC6   					I2C_ADDRESS_IC6_DATASHEET << 1
#define I2C_ADDRESS_IC7   					I2C_ADDRESS_IC7_DATASHEET << 1

//  Command Bytes -- Send to control register in the TCA9554A
#define CMD_REG_INPUT   						0x00
#define CMD_REG_OUTPUT  						0x01
#define CMD_REG_POL_INV 						0x02
#define CMD_REG_CONFIG  						0x03



// ================================================================
// ===       	Define Communication Commands (IMU Module)        ===
// ================================================================

#define IMU_SENSOR_MODULE_GET_STATUS               0x30

#define IMU_SENSOR_MODULE_SEND_BATTERY_VOLTAGE     0x40
#define IMU_SENSOR_MODULE_GET_BATTERY_VOLTAGE      0x41
#define IMU_SENSOR_MODULE_GET_BATTERY_LOW_ERROR    0x42

#define IMU_SENSOR_MODULE_SEND_START_SYNC          0x60
#define IMU_SENSOR_MODULE_GET_SYNC_DONE            0x61

#define IMU_SENSOR_MODULE_SEND_START_CALIBRATION   0x70
#define IMU_SENSOR_MODULE_GET_CANNOT_CALIBRATE     0x71
#define IMU_SENSOR_MODULE_GET_CALIBRATION_DONE     0x72
#define IMU_SENSOR_MODULE_GET_NEED_TO_CALIBRATE    0x73




#define USER_BUTTON_Pin GPIO_PIN_2			// PF2
#define USER_BUTTON_GPIO_Port GPIOF			// PF2
#define USER_BUTTON_EXTI_IRQn EXTI2_IRQn	// PF2 EXTI2_IRQn

#define STCC_CHARGING_Pin GPIO_PIN_3		// PF3
#define STCC_CHARGING_GPIO_Port GPIOF		// PF3

#define STCC_FAULT_Pin GPIO_PIN_4			// PF4
#define STCC_FAULT_GPIO_Port GPIOF			// PF4

#define STCC_EN_Pin GPIO_PIN_5				// PF5
#define STCC_EN_GPIO_Port GPIOF				// PF5

#define LED_GOOD_Pin GPIO_PIN_0				// PB0
#define LED_GOOD_GPIO_Port GPIOB			// PB0

#define LED_ERROR_Pin GPIO_PIN_14			// PB14
#define LED_ERROR_GPIO_Port GPIOB			// PB14

#define SD_ON_OFF_Pin GPIO_PIN_8			// PA8
#define SD_ON_OFF_GPIO_Port GPIOA			// PA8

#define SD_DETECT_Pin GPIO_PIN_9			// PA9
#define SD_DETECT_GPIO_Port GPIOA			// PA9

#define I2C1_INT_Pin GPIO_PIN_5				// PB5
#define I2C1_INT_GPIO_Port GPIOB			// PB5
#define I2C1_INT_EXTI_IRQn EXTI9_5_IRQn		// PB5 EXTI9_5_IRQn

#define LED_BUSY_Pin GPIO_PIN_7				// PB7
#define LED_BUSY_GPIO_Port GPIOB			// PB7



/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
