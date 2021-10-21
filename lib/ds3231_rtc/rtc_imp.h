/**
 * @file rtc_imp.h
 * @brief RTC Implementation
 * @author Alexis.C, Ali.O - Yncrea HdF
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef RTC_RTC_IMP_H_
#define RTC_RTC_IMP_H_

void setTime(char hour, char min,   char sec);
void setDate(char year, char month, char day);

void getTime(char * hour, char * min,   char * sec);
void getDate(char * year, char * month, char * day);

void showTime(char* showtime, unsigned int *cx);

/* Retrieve time from external RTC, and synchronize the internal RTC of the STM32 with the retrieved time */
void setInternalRtc();

#endif /* RTC_RTC_IMP_H_ */
