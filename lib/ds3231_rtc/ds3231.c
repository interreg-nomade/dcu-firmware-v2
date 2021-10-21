/*
 * @file ds3231.c
 * @brief DS3231 RTC Library
 * @author Alexis.C, Ali.O
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

//#include "stm32h7xx_hal_rtc.h"
#include <time.h>
#include "ds3231.h"
#include "usart.h"
/* Contains protected I2C operations */
#include "../../pinterface/pI2C.h"

//TODO : make this function return a boolean
void ds3231_Set_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year)
{
	uint8_t set_time[7];
	set_time[0] = DecToBcd(sec);
	set_time[1] = DecToBcd(min);
	set_time[2] = DecToBcd(hour);
	set_time[3] = DecToBcd(dow);
	set_time[4] = DecToBcd(dom);
	set_time[5] = DecToBcd(month);
	set_time[6] = DecToBcd(year);

	/* I2C operation */
//!	I2C_WriteBytes(I2C_0, DS3231_ADDRESS, DS3231_REG_SECONDS, set_time, 7);
	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, DS3231_REG_SECONDS, 1, set_time, 7, 1000);

	/*##-3- Wait for the end of the transfer #################################*/
	/*  Before starting a new communication transfer, you need to check the current
	        state of the peripheral; if its busy you need to wait for the end of current
	        transfer before starting a new one.
	        For simplicity reasons, this example is just waiting till the end of the
	        transfer, but application may perform other tasks while transfer operation
	        is ongoing. */
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);  // TODO : refactoring this part
}

// TODO : make this function return a boolean
void ds3231_Get_Time(ds3231_time_t * time)
{
	uint8_t get_time[7];
//!	I2C_ReadBytes(I2C_0, DS3231_ADDRESS, DS3231_REG_SECONDS, get_time, 7);
	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, DS3231_REG_SECONDS, 1, get_time, 7, 1000);
	/* Wait the end of the reception */
	while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY);

	time->seconds = 	BcdToDec(get_time[0]);
	time->minutes = 	BcdToDec(get_time[1]);
	time->hours = 		BcdToDec(get_time[2]);
	time->dayofweek = 	BcdToDec(get_time[3]);
	time->dayofmonth = 	BcdToDec(get_time[4]);
	time->month = 		BcdToDec(get_time[5]);
	time->year = 		BcdToDec(get_time[6]);

	/* Calculation of the unixtime - not correctly calculated todo!!! */
	time->unixtime = time_To_unixtime(time->dayofmonth, time->hours, time->minutes, time->seconds);
	time->unixtime += 946681200;
}

uint64_t time_To_unixtime(uint8_t days, uint8_t hours, uint8_t minutes, uint8_t seconds)
{
  return ((days * 24 + hours) * 60 + minutes) * 60 + seconds;
}

// Convert normal decimal numbers to binary coded decimal
uint8_t DecToBcd(uint8_t val)
{
  return (uint8_t)( (val/10*16) + (val%10) );
}

// Convert binary coded decimal to normal decimal numbers
uint8_t BcdToDec(uint8_t val)
{
  return (int)( (val/16*10) + (val%16) );
}
