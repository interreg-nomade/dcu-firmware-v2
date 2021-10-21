/*
 * @file ds3231.h
 * @brief DS3231 RTC Library
 * @author Alexis.C, Ali.O
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef DS3231_H_
#define DS3231_H_

#include <stdbool.h>
#include <stdio.h>

/* Contains protected I2C operations */
#include "../../pinterface/pI2C.h"

/* I2C DS3231's address definition */
#ifndef DS3231_ADDRESS
#define DS3231_ADDRESS        (0xD0)
#endif

/* Internal registers of DS3231 */
#define DS3231_REG_SECONDS			0x00
#define DS3231_REG_MINUTES			0x01
#define DS3231_REG_HOURS			0x02
#define DS3231_REG_DAY				0x03
#define DS3231_REG_DATE				0x04
#define DS3231_REG_MONTH_CENTURY 	0x05
#define DS3231_REG_YEAR				0x06

#define DS3231_REG_ALARM1_SEC		0x07
#define DS3231_REG_ALARM1_MIN		0x08
#define DS3231_REG_ALARM1_HOURS		0x09
#define DS3231_REG_ALARM1_DAY_DATE	0x0A

#define DS3231_REG_ALARM2_MIN		0x0B
#define DS3231_REG_ALARM2_HOURS		0x0C
#define DS3231_REG_ALARM2_DAY_DATE	0x0D

#define DS3231_REG_CONTROL			0x0E
#define DS3231_REG_CONTROL_STATUS	0x0F
#define DS3231_REG_AGING_OFFSET		0x10

#define DS3231_REG_MSB_TEMP			0x11
#define DS3231_REG_LSB_TEMP			0x12

#define AM_PM     (1 << 5)
#define MODE      (1 << 6)
#define DY_DT     (1 << 6)
#define ALRM_MASK (1 << 7)

//control register bit masks
#define A1IE  (1 << 0)
#define A2IE  (1 << 1)
#define INTCN (1 << 2)
#define RS1   (1 << 3)
#define RS2   (1 << 4)
#define CONV  (1 << 5)
#define BBSQW (1 << 6)
#define EOSC  (1 << 7)

//status register bit masks
#define A1F     (1 << 0)
#define A2F     (1 << 1)
#define BSY     (1 << 2)
#define EN32KHZ (1 << 3)
#define OSF     (1 << 7)

//convenience macros to convert to and from tm years
#define  tmYearToCalendar(Y) ((Y) + 1970)  // full four digit year
#define  CalendarYrToTm(Y)   ((Y) - 1970)
#define  tmYearToY2k(Y)      ((Y) - 30)    // offset is from 2000
#define  y2kYearToTm(Y)      ((Y) + 30)

/**
* ds3231_time_t - Struct for containing time data.
*
* Members:
*
* - uint32_t seconds - Use decimal value. Member fx's convert to BCD
*
* - uint32_t minutes - Use decimal value. Member fx's convert to BCD
*
* - uint32_t hours   - Use decimal value. Member fx's convert to BCD
*
* - bool am_pm      - TRUE for PM, same logic as datasheet
*
* - bool mode       - TRUE for 12 hour, same logic as datasheet
*/
typedef struct
{
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t dayofweek;
	uint8_t dayofmonth;
	uint8_t month;
	uint8_t year;
	uint64_t unixtime;
    bool am_pm;
    bool mode;
}ds3231_time_t;

/**
* ds3231_calendar_t - Struct for containing calendar data.
*
* Members:
*
* - uint32_t day   - Use decimal value. Member fx's convert to BCD
*
* - uint32_t date  - Use decimal value. Member fx's convert to BCD
*
* - uint32_t month - Use decimal value. Member fx's convert to BCD
*
* - uint32_t year  - Use decimal value. Member fx's convert to BCD
*/
typedef struct
{
    uint32_t day;
    uint32_t date;
    uint32_t month;
    uint32_t year;
}ds3231_calendar_t;

//uint16_t ds3231_set_time(ds3231_time_t time);
//uint16_t ds3231_set_calendar(ds3231_calendar_t calendar);
/* Low bottom functions */
//bool ds3231_read_datetime(uint8_t* dest); /* dest must be >= 7 bytes ! */
//bool ds3231_write_register(uint8_t reg, uint8_t value); /*TODO in case of time reactualisation using gps time*/
//
//void ds3231_refresh_time();
//
//void ds3231_get_datetime(RTC_TimeTypeDef* time, RTC_DateTypeDef* date);
//void ds3231_get_elapsed_secs_since_2000(uint32_t * secs);
//
//unsigned int enableRtcOscillator();

void ds3231_Set_Time(uint8_t sec, uint8_t min, uint8_t hour, uint8_t dow, uint8_t dom, uint8_t month, uint8_t year);
void ds3231_Get_Time(ds3231_time_t * time);
uint64_t time_To_unixtime(uint8_t days, uint8_t hours, uint8_t minutes, uint8_t seconds);
uint8_t DecToBcd(uint8_t val);
uint8_t BcdToDec(uint8_t val);


#endif /* DS3231_H_ */
