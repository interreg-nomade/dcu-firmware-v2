/**
 * @file app_rtc.h
 * @brief RTC RTOS Application
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date 23 August 2019
 *
 * The purpose is to have an accurate 1ms ticker running based on the boot time
 *
 */
#ifndef APP_RTC_H_
#define APP_RTC_H_

#include <stdint.h>

extern unsigned int rtcUnsynchronized;
extern uint64_t epochtime_ms;

void rtos_rtc_thread_init();
void rtos_rtc_increment(); // to call in a 1ms period timer
void app_rtc_get_unix_epoch_ms(uint64_t * dest);
void app_rtc_set_unix_epoch_ms(uint64_t epoch_AD_ms);
uint64_t app_rtc_get_unix_epoch   (void);
void app_rtc_set_cycle_counter(uint32_t cc);
uint32_t app_rtc_get_cycle_counter();
void app_rtc_calculate_cycle_counter_from_reference(uint64_t reference);
void app_rtc_inc_cycle_counter(void);

unsigned int app_rtc_get_rtcUnsyncState(void);
void app_rtc_set_rtUnsyncState(unsigned int state);

void app_rtc_print_RTCdateTime(void);
void compare_RTCunixtime_Epoch(void);

#endif /* APP_RTC_H_ */
