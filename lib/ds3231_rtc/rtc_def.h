/**
 * @file rtc_def.h
 * @brief DS3231 Library definitions
 * @author Alexis.C, Ali.O - Yncrea HdF
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef RTC_RTC_DEF_H_
#define RTC_RTC_DEF_H_

typedef struct {
    /* All the variables are in binary mode, not BCD. */
    unsigned int year;
    unsigned int month;
    unsigned int day;

    unsigned int hour;
    unsigned int min;
    unsigned int sec;

} date_time_t;

#endif /* RTC_RTC_DEF_H_ */
