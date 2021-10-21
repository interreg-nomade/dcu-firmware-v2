/**
 * @file board.c
 * @brief Board interface file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date June 2019
 *
 * The purpose is to have functions related to the board e.g. reading pins, turning leds on, etc.
 *
 */
#include "board.h"
#include "gpio.h"

void Led_Toggle(unsigned int nled)
{
	switch (nled)
	{
		case 1:
		{
//!!			HAL_GPIO_TogglePin(USR_LED1_GPIO_Port, USR_LED1_GPIO_Pin);
			break;
		}
		case 2:
		{
//!!			HAL_GPIO_TogglePin(USR_LED2_GPIO_Port, USR_LED2_GPIO_Pin);
			break;
		}
		case 3:
		{
//!!			HAL_GPIO_TogglePin(USR_LED3_GPIO_Port, USR_LED3_GPIO_Pin);
			break;
		}
		case 4:
		{
//!!			HAL_GPIO_TogglePin(USR_LED4_GPIO_Port, USR_LED4_GPIO_Pin);
			break;
		}
		case 5:
		{
//!!			HAL_GPIO_TogglePin(USR_LED5_GPIO_Port, USR_LED5_GPIO_Pin);
			break;
		}
	}

}

void Led_TurnOn(unsigned int nled)
{
	switch (nled)
	{
		case 1:
		{
//!!			HAL_GPIO_WritePin(USR_LED1_GPIO_Port, USR_LED1_GPIO_Pin , GPIO_PIN_SET);
			break;
		}
		case 2:
		{
//!!			HAL_GPIO_WritePin(USR_LED2_GPIO_Port, USR_LED2_GPIO_Pin , GPIO_PIN_SET);
			break;
		}
		case 3:
		{
//!!			HAL_GPIO_WritePin(USR_LED3_GPIO_Port, USR_LED3_GPIO_Pin , GPIO_PIN_SET);
			break;
		}
		case 4:
		{
//!!			HAL_GPIO_WritePin(USR_LED4_GPIO_Port, USR_LED4_GPIO_Pin , GPIO_PIN_SET);
			break;
		}
		case 5:
		{
//!!			HAL_GPIO_WritePin(USR_LED5_GPIO_Port, USR_LED5_GPIO_Pin , GPIO_PIN_SET);
			break;
		}
	}
}

void Led_TurnOff(unsigned int nled)
{
	switch (nled)
	{
		case 1:
		{
//!!			HAL_GPIO_WritePin(USR_LED1_GPIO_Port, USR_LED1_GPIO_Pin , GPIO_PIN_RESET);
			break;
		}
		case 2:
		{
//!!			HAL_GPIO_WritePin(USR_LED2_GPIO_Port, USR_LED2_GPIO_Pin , GPIO_PIN_RESET);
			break;
		}
		case 3:
		{
//!!			HAL_GPIO_WritePin(USR_LED3_GPIO_Port, USR_LED3_GPIO_Pin , GPIO_PIN_RESET);
			break;
		}
		case 4:
		{
//!!			HAL_GPIO_WritePin(USR_LED4_GPIO_Port, USR_LED4_GPIO_Pin , GPIO_PIN_RESET);
			break;
		}
		case 5:
		{
//!!			HAL_GPIO_WritePin(USR_LED5_GPIO_Port, USR_LED5_GPIO_Pin , GPIO_PIN_RESET);
			break;
		}
	}
}

void FTDI_TurnOff()
{
//!!	HAL_GPIO_WritePin(FTDI_ENABLE_GPIO_Port, FTDI_ENABLE_GPIO_Pin , GPIO_PIN_RESET);
}

void FTDI_TurnOn()

{
//!!	HAL_GPIO_WritePin(FTDI_ENABLE_GPIO_Port, FTDI_ENABLE_GPIO_Pin , GPIO_PIN_SET);
}

void AndroidLinkBoard_TurnOff()
{
//!!	HAL_GPIO_WritePin(SPI4_GPIO_Port, SPI4_GPIO_Pin , GPIO_PIN_RESET);
}

void AndroidLinkBoard_TurnOn()
{
//!!	HAL_GPIO_WritePin(SPI4_GPIO_Port, SPI4_GPIO_Pin , GPIO_PIN_SET);
}

unsigned int STCC_FAULT_Read()
{
//!!	return (unsigned int) (HAL_GPIO_ReadPin(STCC_FAULT_GPIO_Port, STCC_FAULT_GPIO_Pin));
	unsigned int res = 0;
	return res;

}

PWC_HW_TYPE PWC_ReadType()
{
//!!	GPIO_PinState c1 = HAL_GPIO_ReadPin(C1_GPIO_Port, C1_GPIO_Pin);
//!!	GPIO_PinState c2 = HAL_GPIO_ReadPin(C2_GPIO_Port, C2_GPIO_Pin);
	unsigned int res = 0;
//!!	res = (c2 << 1) | c1;
	return res;
}

void SD_TurnOn(void)
{
	HAL_GPIO_WritePin(SD_ON_OFF_GPIO_Port, SD_ON_OFF_GPIO_Pin, GPIO_PIN_SET);
}
void SD_TurnOff(void)
{
	HAL_GPIO_WritePin(SD_ON_OFF_GPIO_Port, SD_ON_OFF_GPIO_Pin, GPIO_PIN_RESET);
}

void AuxPow_TurnOn(void)
{
//!!	HAL_GPIO_WritePin(EN_SENSOR_RAIL_GPIO_Port, EN_SENSOR_RAIL_GPIO_Pin, GPIO_PIN_SET);
}

void AuxPow_TurnOff(void)
{
//!!	HAL_GPIO_WritePin(EN_SENSOR_RAIL_GPIO_Port, EN_SENSOR_RAIL_GPIO_Pin, GPIO_PIN_RESET);
}

