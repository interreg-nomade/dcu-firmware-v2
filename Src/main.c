/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"
#include "fatfs.h"
#include "i2c.h"
#include "mdma.h"
#include "rtc.h"
#include "sdmmc.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include <string.h>
#include "bsp_driver_sd.h"
#include "usb_com.h"
#include "imu_com.h"
#include "UartRingbuffer.h"
#include "UartRingbufferManager.h"
#include "uart_com.h"
#include "math.h"
#include "interface_sd.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */


#include "board.h"

#include "timer_callback.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

FATFS myFATAFS = {0};
FIL myFILE;
UINT testByte;

char string[150];
QueueHandle_t pPrintQueue; // print strings via queue and gatekeeper function
static osThreadId pPrintTaskHandle;

float PI = 3.14159265358979323846;

imu_module imu_1 = {1, &huart4, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 1", 0, 0, 0, 0, 0, 0, 0, 0 };
imu_module imu_2 = {2, &huart6, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 2", 0, 0, 0, 0, 0, 0, 0, 0 };
imu_module imu_3 = {3, &huart7, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 3", 0, 0, 0, 0, 0, 0, 0, 0 };
imu_module imu_4 = {4, &huart8, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 4", 0, 0, 0, 0, 0, 0, 0, 0 };
imu_module imu_5 = {5, &huart1, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 5", 0, 0, 0, 0, 0, 0, 0, 0 };
imu_module imu_6 = {6, &huart2, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, "BT Module 6", 0, 0, 0, 0, 0, 0, 0, 0 };

imu_module *imu_array [] = {&imu_1, &imu_2, &imu_3, &imu_4, &imu_5, &imu_6};

//uint16_t file_nummer = 0;

uint8_t gpio_buf_IC5 = 0x00;

int num_send_1 = 0, num_send_2 = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
static void SWD_Init(void);
static void MPU_Config(void);
static void pPrintGatekeeperThread(void);

/**********IOExpander**********/
static void startUpLeds(void);
static void IOExpander_init(I2C_HandleTypeDef *hi2c, uint8_t address);
static void IOExpander_set(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t io);
static void IOExpander_clear(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t io);
static void IOExpander_clearAll(I2C_HandleTypeDef *hi2c, uint8_t address);
static void IOExpander_update(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t buf);

/*********Calculations********/
static void convertBuffer(uint8_t * buf, uint8_t sensor_number);
static void getYawPitchRoll(int16_t *data, float *newdata);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */
  MPU_Config();
  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  SWD_Init(); /* Fixes the SWD issue on this STM32H7 */
  /* Enable SRAM 1, 2 and 3) */
  /* Register AHB2ENR informations */
  /* Bit 31: Enable SRAM3 (1: Enabled, 0:Disabled) */
  /* Bit 30: Enable SRAM2 (1: Enabled, 0:Disabled) */
  /* Bit 29: Enable SRAM1 (1: Enabled, 0:Disabled) */
  /* Note: done in startup instea d */
  RCC->AHB2ENR |= (0x7 << 29);
  HAL_Delay(1);
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SDMMC1_SD_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();

  /* USER CODE BEGIN 2 */

  /* Start timer 2 */
  tim2_start();

  /* Start leds */
  //startUpLeds();
  HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);

#if RTT_DBG_TEST
  for (unsigned int i = 0; i<30; i++)
  {
	  //SEGGER_RTT_WriteString(0, "Test\n");
	  printf("Test %d\n", i);
	  HAL_Delay(500);
  }
#endif

  /* USER CODE END 2 */

  /* Call init function for freertos objects (in freertos.c) */
  MX_FREERTOS_Init();

  uint16_t software_version = DCU_SW_VERSION;
  sprintf(string, "************************************\n   NOMADe Mainboard V3.1\n************************************\n");
  HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);

//  pPrintQueue = xQueueCreate(10, sizeof(char *)); // protected print queue can contain max 10 character pointers
  pPrintQueue = xQueueCreate(30, sizeof(string)); // protected print queue can contain max 30 strings of 150 characters each
  osThreadDef(pPrintGatekeeper, pPrintGatekeeperThread, osPriorityNormal, 0, 1000);
  pPrintTaskHandle = osThreadCreate(osThread(pPrintGatekeeper), NULL);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Supply configuration update enable
  */
  MODIFY_REG(PWR->CR3, PWR_CR3_SCUEN, 0);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
  while ((PWR->D3CR & (PWR_D3CR_VOSRDY)) != PWR_D3CR_VOSRDY) {}

  /** Macro to configure the PLL clock source
  */
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV4; //DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI; //HSE; // RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2; //32;
  RCC_OscInitStruct.PLL.PLLN = 100; //129;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 20; //2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_1; //3; //RCC_PLL1VCIRANGE_1;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK; // RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2; //DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2; //DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2; //DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2; //DIV1;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2; //DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3|RCC_PERIPHCLK_USART2
                              |RCC_PERIPHCLK_UART4|RCC_PERIPHCLK_UART7
                              |RCC_PERIPHCLK_USART6|RCC_PERIPHCLK_USART1
                              |RCC_PERIPHCLK_UART8|RCC_PERIPHCLK_UART5
                              |RCC_PERIPHCLK_SDMMC|RCC_PERIPHCLK_I2C1;

  PeriphClkInitStruct.PLL3.PLL3M = 16;
  PeriphClkInitStruct.PLL3.PLL3N = 258; //512;
  PeriphClkInitStruct.PLL3.PLL3P = 4;
  PeriphClkInitStruct.PLL3.PLL3Q = 8; //2;
  PeriphClkInitStruct.PLL3.PLL3R = 2;
  PeriphClkInitStruct.PLL3.PLL3RGE = RCC_PLL3VCIRANGE_0;
  PeriphClkInitStruct.PLL3.PLL3VCOSEL = RCC_PLL3VCOWIDE;
  PeriphClkInitStruct.PLL3.PLL3FRACN = 0;

  PeriphClkInitStruct.SdmmcClockSelection = RCC_SDMMCCLKSOURCE_PLL;
  PeriphClkInitStruct.Usart234578ClockSelection = RCC_USART234578CLKSOURCE_PLL3; //D2PCLK1;
  PeriphClkInitStruct.Usart16ClockSelection = RCC_USART16CLKSOURCE_PLL3;
  PeriphClkInitStruct.I2c123ClockSelection = RCC_I2C123CLKSOURCE_D2PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
void SWD_Init(void)
{
  *(__IO uint32_t*)(0x5C001004) |= 0x00700000; // DBGMCU_CR D3DBGCKEN D1DBGCKEN TRACECLKEN

  //UNLOCK FUNNEL
  *(__IO uint32_t*)(0x5C004FB0) = 0xC5ACCE55; // SWTF_CTRL
  *(__IO uint32_t*)(0x5C003FB0) = 0xC5ACCE55; // SWO_LAR

  //SWO current output divisor register
  //This divisor value (0x000000C7) corresponds to 400Mhz
  //To change it, you can use the following rule
  // value = (CPU Freq/sw speed )-1
   *(__IO uint32_t*)(0x5C003010) = (*(__IO uint32_t*)(0x5C003010) & 0xfffff000) | ((SystemCoreClock / 2000000) - 1); // SWO_CODR

  //SWO selected pin protocol register
   *(__IO uint32_t*)(0x5C0030F0) = 0x00000002; // SWO_SPPR

  //Enable ITM input of SWO trace funnel
   *(__IO uint32_t*)(0x5C004000) |= 0x00000001; // SWFT_CTRL

  //RCC_AHB4ENR enable GPIOB clock
   *(__IO uint32_t*)(0x580244E0) |= 0x00000002;

  // Configure GPIOB pin 3 as AF
   *(__IO uint32_t*)(0x58020400) = (*(__IO uint32_t*)(0x58020400) & 0xffffff3f) | 0x00000080;

  // Configure GPIOB pin 3 Speed
   *(__IO uint32_t*)(0x58020408) |= 0x00000080;

  // Force AF0 for GPIOB pin 3
   *(__IO uint32_t*)(0x58020420) &= 0xFFFF0FFF;
}

static void MPU_Config(void)
{
	  MPU_Region_InitTypeDef MPU_InitStruct;

	  /* Disable the MPU */
	  HAL_MPU_Disable();

	  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
	  MPU_InitStruct.BaseAddress = 0x24000000;
	  MPU_InitStruct.Size = MPU_REGION_SIZE_512KB;
	  MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
	  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
	  MPU_InitStruct.IsCacheable = MPU_ACCESS_CACHEABLE;
	  MPU_InitStruct.IsShareable = MPU_ACCESS_NOT_SHAREABLE;
	  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
	  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
	  MPU_InitStruct.SubRegionDisable = 0x00;
	  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_ENABLE;

	  HAL_MPU_ConfigRegion(&MPU_InitStruct);

	  /* Enable the MPU */
	  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}


static void pPrintGatekeeperThread(void)
{
//	char *pMessageToPrint; // for Queue of pointers
	char MessageToPrint[150]; // for Queue of strings
	for (;;)
	{
//		xQueueReceive(pPrintQueue, &pMessageToPrint, portMAX_DELAY); // for Queue of pointers
		xQueueReceive(pPrintQueue, MessageToPrint, portMAX_DELAY); // for Queue of strings
		HAL_UART_Transmit(&huart5, (uint8_t *)MessageToPrint, strlen(MessageToPrint), 25);
	}
}
/* USER CODE END 4 */

#define SAVE_QUARTERNIONS_SD_CARD

void convertBuffer(uint8_t * buf, uint8_t sensor_number){

	uint32_t timestamp;
	uint16_t pakket_send_nr;

  #ifdef SAVE_SD_CARD
    uint16_t sd_card_buffer [20][3];
  #endif

  #ifdef SAVE_YPR_SD_CARD
    uint8_t sd_card_buffer [NUMBER_OF_DATA_READS_IN_BT_PACKET * 6];
  #endif

	#ifdef SAVE_QUARTERNIONS_SD_CARD
		int16_t sd_card_buffer [NUMBER_OF_DATA_READS_IN_BT_PACKET * 4];
  #endif

  for(uint8_t j = 0; j < NUMBER_OF_BT_PACKETS; j++){
    uint16_t start_pos = PACKET_START_POS + 7;
		// 1e byte: 					command "IMU_SENSOR_MODULE_REQ_SEND_DATA"
		// 2e & 3e byte:			packet_send_nr
		// 3e 4e 5e 6e byte:	timestamp

		pakket_send_nr = buf[PACKET_START_POS + 1] | (buf[PACKET_START_POS + 2] << 8);
		timestamp = buf[PACKET_START_POS + 3] | (buf[PACKET_START_POS + 4] << 8) | (buf[PACKET_START_POS + 5] << 16) | (buf[PACKET_START_POS + 6] << 24);

    for(int i = 0; i < NUMBER_OF_DATA_READS_IN_BT_PACKET; i++){
      int16_t data [4];
      data[0] = ((buf[start_pos + 0 + 8 * i] << 8) | buf[start_pos + 1 + 8 * i]);
      data[1] = ((buf[start_pos + 2 + 8 * i] << 8) | buf[start_pos + 3 + 8 * i]);
      data[2] = ((buf[start_pos + 4 + 8 * i] << 8) | buf[start_pos + 5 + 8 * i]);
      data[3] = ((buf[start_pos + 6 + 8 * i] << 8) | buf[start_pos + 7 + 8 * i]);

      //uint8_t tx_send [8] = {data[0] >> 8, (uint8_t)data[0], data[1] >> 8, (uint8_t)data[1], data[2] >> 8, (uint8_t)data[2], data[3] >> 8, (uint8_t)data[3]};
      //HAL_UART_Transmit(&huart3, tx_send, 8, 25);

			/*
//			char string [100];
			sprintf(string, "%d %d %d %d\n", data[0], data[1], data[2], data[3]);
				//sprintf(string, "Timestamp: %i - Yaw: %i | Pitch: %i | Roll: %i - %i\n", timestamp, (uint16_t)(buf_YPR[0]/PI*180+180), (uint16_t)(buf_YPR[1]/PI*180+180), (uint16_t)(buf_YPR[2]/PI*180+180), num_send_1);
			UART_COM_write(&huart5, (uint8_t *)string, strlen(string));
			*/

			#ifdef SAVE_YPR_SD_CARD
				float buf_YPR [3];
				getYawPitchRoll(data, buf_YPR);
      #endif


      //    ***   Option 1: Visual    ***   //
      #ifdef SEND_DATA_VISUAL
//        char string [100];
        //sprintf(string, "Data 0: %.2i | Data 1: %.2i | Data 2: %.2i | Data 3: %.2i\n", data[0], data[1], data[2], data[3]);
        //sprintf(string, "Yaw: %.2f | Pitch: %.2f | Roll: %.2f\n", (buf_YPR[0]/PI*180+180), (buf_YPR[1]/PI*180+180), (buf_YPR[2]/PI*180+180));
        //sprintf(string, "Yaw: %i | Pitch: %i | Roll: %i\n", (uint16_t)(buf_YPR[0]/PI*180+180), (uint16_t)(buf_YPR[1]/PI*180+180), (uint16_t)(buf_YPR[2]/PI*180+180));
				sprintf(string, "Yaw: %i | Pitch: %i | Roll: %i, %i, %i, %i\n", (uint16_t)(buf_YPR[0]/PI*180+180), (uint16_t)(buf_YPR[1]/PI*180+180), (uint16_t)(buf_YPR[2]/PI*180+180), num_send_1, num_send_2, timestamp);
				//sprintf(string, "Timestamp: %i - Yaw: %i | Pitch: %i | Roll: %i - %i\n", timestamp, (uint16_t)(buf_YPR[0]/PI*180+180), (uint16_t)(buf_YPR[1]/PI*180+180), (uint16_t)(buf_YPR[2]/PI*180+180), num_send_1);
				UART_COM_write(&huart5, (uint8_t *)string, strlen(string));
      #endif
      //    ***   Option 2: Pycharm frame YPR   ***   //
      #ifdef SEND_DATA_YPR
        uint16_t data_hex_16 [3] = {(uint16_t)(buf_YPR[0]/PI*180+180), (uint16_t)(buf_YPR[1]/PI*180+180), (uint16_t)(buf_YPR[2]/PI*180+180)};
        uint8_t data_hex_buf [10];
        data_hex_buf [0] = 0x02;
        data_hex_buf [1] = 0x0A;
        data_hex_buf [2] = 0x00;
        data_hex_buf [3] = (uint8_t)(data_hex_16 [0]);
        data_hex_buf [4] = (uint8_t)(data_hex_16 [0] >> 8);
        data_hex_buf [5] = (uint8_t)(data_hex_16 [1]);
        data_hex_buf [6] = (uint8_t)(data_hex_16 [1] >> 8);
        data_hex_buf [7] = (uint8_t)(data_hex_16 [2]);
        data_hex_buf [8] = (uint8_t)(data_hex_16 [2] >> 8);
        data_hex_buf [9] = calculateCS(data_hex_buf, 9);

        HAL_UART_Transmit(&huart5, data_hex_buf, sizeof(data_hex_buf), 25);
      #endif

      //    ***   Option 3: Pycharm frame QUATERNIONS   ***   //
      #ifdef SEND_DATA_QUATERNIONS
        uint8_t data_hex_buf [12];
        data_hex_buf [0]  = 0x02;
        data_hex_buf [1]  = 0x0A;
        data_hex_buf [2]  = 0x00;
        data_hex_buf [3]  = (uint8_t)(data [0]);
        data_hex_buf [4]  = (uint8_t)(data [0] >> 8);
        data_hex_buf [5]  = (uint8_t)(data [1]);
        data_hex_buf [6]  = (uint8_t)(data [1] >> 8);
        data_hex_buf [7]  = (uint8_t)(data [2]);
        data_hex_buf [8]  = (uint8_t)(data [2] >> 8);
        data_hex_buf [9]  = (uint8_t)(data [3]);
        data_hex_buf [10] = (uint8_t)(data [3] >> 8);
        data_hex_buf [11] = calculateCS(data_hex_buf, 9);

        HAL_UART_Transmit(&huart5, data_hex_buf, sizeof(data_hex_buf), 25);
      #endif

      //    ***   Option 4: Save YPR on SD card   ***   //
      #ifdef SAVE_SD_CARD
        sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i][0] = (uint16_t)(buf_YPR[0]/PI*180+180);
        sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i][1] = (uint16_t)(buf_YPR[1]/PI*180+180);
        sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i][2] = (uint16_t)(buf_YPR[2]/PI*180+180);
      #endif

			      //    ***   Option 5: Save YPR on SD card  --NEW--  ***   //
      #ifdef SAVE_YPR_SD_CARD
				for(uint8_t k = 0; k < 3; k++){
					uint16_t value = (uint16_t)(buf_YPR[k]/PI*180+180);
					sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i*6 + 2 * k] 			= (uint8_t) value;
					sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i*6 + 2 * k + 1] 	= (uint8_t) value >> 8;
				}
			#endif

			#ifdef SAVE_QUARTERNIONS_SD_CARD
				for(uint8_t k = 0; k < 4; k++){
					sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i*4 + k] = data[k];
				}
			#endif
    }
  }

  //    ***   Option 4: Save YPR on SD card   ***   //
  #ifdef SAVE_SD_CARD
    HAL_GPIO_TogglePin(LED_BUSY_GPIO_Port, LED_BUSY_Pin);
    char path [25];
    sprintf(path, "MET_%d.TXT", file_nummer);
    f_open(&myFILE, path, FA_OPEN_APPEND | FA_WRITE);
    for(int i = 0; i < SIZE_SD_CARD_READ_BUF; i++){
      f_printf(&myFILE, "%d,%d,%d,%d,%d\n", timestamp, sensor_number, (uint16_t)(sd_card_buffer[i][0]), (uint16_t)(sd_card_buffer[i][1]), (uint16_t)(sd_card_buffer[i][2]));
    }
    f_close(&myFILE);
    HAL_GPIO_TogglePin(LED_BUSY_GPIO_Port, LED_BUSY_Pin);
  #endif


  //    ***   Option 5: Save YPR on SD card  --NEW-- 	***   //
  #ifdef SAVE_YPR_SD_CARD
    //char path [25];
    //sprintf(path, "MET_%d.TXT", file_nummer);
    //f_open(&myFILE, path, FA_OPEN_APPEND | FA_WRITE);
		SD_CARD_COM_save_data(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
    /*
		for(int i = 0; i < SIZE_SD_CARD_READ_BUF; i++){
			SD_CARD_COM_save_data(timestamp, sensor_number, &sd_card_buffer);
      f_printf(&myFILE, "%d,%d,%d,%d,%d\n", timestamp, sensor_number, (uint16_t)(sd_card_buffer[i][0]), (uint16_t)(sd_card_buffer[i][1]), (uint16_t)(sd_card_buffer[i][2]));
    }
		*/
    //f_close(&myFILE);
  #endif


	#ifdef SAVE_QUARTERNIONS_SD_CARD
		//SD_CARD_COM_save_data(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
  #endif
}



/********************************************************************************************************************/

/*****************************************************************************************************/


void getYawPitchRoll(int16_t *data, float *newdata){
    //  q = quaternion
  float q [4];
  q[0] = (float)data[0] / 16384.0f;   //  w
  q[1] = (float)data[1] / 16384.0f;   //  x
  q[2] = (float)data[2] / 16384.0f;   //  y
  q[3] = (float)data[3] / 16384.0f;   //  z

  // gravity
  float gravity [3];
  gravity[0] = 2 * (q[1] * q[3] - q[0] * q[2]);                         //  x
  gravity[1] = 2 * (q[0] * q[1] + q[2] * q[3]);                         //  y
  gravity[2] = q[0] * q[0] - q[1] * q[1] - q[2] * q[2] + q[3] * q[3];   //  z

  //  Euler
  //float euler [3];
  //euler[0] = atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0] * q[0] + 2 * q[1] * q[1] - 1);     // psi
  //euler[1] = -asin(2 * q[1] * q[3] + 2 * q[0] * q[2]);                                            // theta
  //euler[2] = atan2(2 * q[2] * q[3] - 2 * q[0] * q[1], 2 * q[0] * q[0] + 2 * q[3] * q[3] - 1);     // phi


  /*
    // calculate gravity vector
  gravity[0] = 2 * (q[1]*q[3] - q[0]*q[2]);
  gravity[1] = 2 * (q[0]*q[1] + q[2]*q[3]);
  gravity[2] = q[0]*q[0] - q[1]*q[1] - q[2]*q[2] + q[3]*q[3];

  // calculate Euler angles
  euler[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
  euler[1] = -asin(2*q[1]*q[3] + 2*q[0]*q[2]);
  euler[2] = atan2(2*q[2]*q[3] - 2*q[0]*q[1], 2*q[0]*q[0] + 2*q[3]*q[3] - 1);

  // calculate yaw/pitch/roll angles
  ypr[0] = atan2(2*q[1]*q[2] - 2*q[0]*q[3], 2*q[0]*q[0] + 2*q[1]*q[1] - 1);
  ypr[1] = atan(gravity[0] / sqrt(gravity[1]*gravity[1] + gravity[2]*gravity[2]));
  ypr[2] = atan(gravity[1] / sqrt(gravity[0]*gravity[0] + gravity[2]*gravity[2]));


  */

  // yaw: (about Z axis)
  newdata[0] = atan2(2 * q[1] * q[2] - 2 * q[0] * q[3], 2 * q[0] * q[0] + 2 * q[1] * q[1] - 1);
  // pitch: (nose up/down, about Y axis)
  newdata[1] = atan2(gravity[0] , sqrt(gravity[1] * gravity[1] + gravity[2] * gravity[2]));
  // roll: (tilt left/right, about X axis)
  newdata[2] = atan2(gravity[1], gravity[2]);

  if (gravity[2] < 0) {
    if(newdata[1] > 0)    newdata[1] = PI   - newdata[1];
    else                  newdata[1] = -PI  - newdata[1];
  }

}

// ========== END BT PACKET HANDLER FUNCTIONS


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
    ticker_1ms_callback();
  }
  /* USER CODE BEGIN Callback 1 */

  else
	if (htim->Instance == htim2.Instance)
  {
	  tim2_callback();
  }
  /* USER CODE END Callback 1 */
}

void startUpLeds(void){
  IOExpander_init(&hi2c1, I2C_ADDRESS_IC5);

  //HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
  HAL_Delay(100);
  for(uint8_t i = 0; i < 6; i++){
    IOExpander_set(&hi2c1, I2C_ADDRESS_IC5, i);
    HAL_Delay(100);
  }
  IOExpander_clearAll(&hi2c1, I2C_ADDRESS_IC5);
  //HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
}


void IOExpander_init(I2C_HandleTypeDef *hi2c, uint8_t address){
  uint8_t send_buf [] = {CMD_REG_CONFIG, 0x00};
  HAL_I2C_Master_Transmit(hi2c, address, send_buf, sizeof(send_buf), 25);
}

void IOExpander_set(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t io){
  gpio_buf_IC5 |= 1 << io;
  IOExpander_update(hi2c, address, gpio_buf_IC5);
}

uint8_t IOExpander_getstate(uint8_t io){
	return ((gpio_buf_IC5 >> io) & 0x01);
}

void IOExpander_toggle(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t io){
  if(IOExpander_getstate(io)) 	IOExpander_clear(hi2c, address, io);
	else													IOExpander_set(hi2c, address, io);
}


void IOExpander_clear(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t io){
  gpio_buf_IC5 &= 0 << io;
  IOExpander_update(hi2c, address, gpio_buf_IC5);
}

void IOExpander_clearAll(I2C_HandleTypeDef *hi2c, uint8_t address){
  gpio_buf_IC5 = 0x00;
  IOExpander_update(hi2c, address, gpio_buf_IC5);
}

void IOExpander_update(I2C_HandleTypeDef *hi2c, uint8_t address, uint8_t buf){
  uint8_t send_buf [] = {CMD_REG_OUTPUT, buf};
  HAL_I2C_Master_Transmit(hi2c, address, send_buf, sizeof(send_buf), 25);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
