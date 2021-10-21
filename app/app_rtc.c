/**
 * @file app_rtc.c
 * @brief RTC RTOS Application
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date 23 August 2019
 *
 * The purpose is to have an accurate 1ms ticker running based on the boot time
 *
 */

#include "app_rtc.h"
#include "app_sync.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "semphr.h"

#include <stdint.h>
#include <stdbool.h>
#include "string.h"
#include <time.h>
#include <inttypes.h>

#include "ds3231_rtc/ds3231.h"
//#include "M24512/m24512.h"
#include "common.h"

#include "stm32h7xx_hal_rtc.h"

#define PRINTF_APP_RTC 1
extern char string[];
extern QueueHandle_t pPrintQueue;

uint64_t * pEpoch64;
static bool incEnable;
static instrument_config_t * RtcInstrumentHandler;
uint64_t epochtime_ms;
static uint32_t cycleCounter;
unsigned int rtcUnsynchronized = 1;

int RTC_Config_Still_Valid(void);
void RTC_Config_Init();
void rtos_rtc_thread(const void * params);

static osThreadId rtcThreadHandle;

void rtos_rtc_thread_init()
{
  osThreadDef(rtcThread, rtos_rtc_thread, osPriorityNormal, 0, 256);
  rtcThreadHandle = osThreadCreate(osThread(rtcThread), NULL);
}

void rtos_rtc_thread(const void * params)
{
  RTC_Config_Init();
#if PRINTF_APP_RTC
  xQueueSend(pPrintQueue, "[app_rtc] [rtos_rtc_thread] RTC_Config_Init() done.\n", 0);
#endif
  /* Check the time */
  //TODO: Store last bootup time in EEPROM and make sure we are after it.
  uint64_t epoch;
  uint64_t epochBootTime;
  uint32_t bootTime;
  ds3231_time_t ds3231time;
  struct tm dateTime;
  char buff[70];
  memset(&ds3231time, 0, sizeof(ds3231_time_t));
  memset(&dateTime, 0, sizeof(dateTime));
  /* Check if the RTC has been synchronized before
   * this information is kept in the eeprom at the address 0x0001 */
  uint8_t data = 0;
//	m24512_epprom_read(0x0001,
//			&data,
//			1);
//	printf("[APP RTC]Read data from EEPPROM %d\n", data);

  if(data == 0xFF)
  {
	rtcUnsynchronized = 1;
  }
  else if(data == 1)
  {
	rtcUnsynchronized = 0;
  }

  // set dummy date
  // todo get date & time out of EEPROM
  ds3231time.seconds    = 15;
  ds3231time.minutes    = 14;
  ds3231time.hours      = 3;
  ds3231time.dayofweek  = 2;
  ds3231time.dayofmonth = 4;
  ds3231time.month      = 5;
  ds3231time.year       = 21; // year - 2000;

//	uint8_t set_time[7];
//	set_time[0] = DecToBcd(ds3231time.seconds);
//	set_time[1] = DecToBcd(ds3231time.minutes);
//	set_time[2] = DecToBcd(ds3231time.hours);
//	set_time[3] = DecToBcd(ds3231time.dayofweek);
//	set_time[4] = DecToBcd(ds3231time.dayofmonth);
//	set_time[5] = DecToBcd(ds3231time.month);
//	set_time[6] = DecToBcd(ds3231time.year);
//	HAL_I2C_Mem_Write(&hi2c1, DS3231_ADDRESS, 0x00, 1, set_time, 7, 1000);
//	osDelay(1000);
//	uint8_t get_time[7];
//	HAL_I2C_Mem_Read(&hi2c1, DS3231_ADDRESS, 0x00, 1, get_time, 7, 1000);
//	dateTime.tm_sec = 	BcdToDec(get_time[0]);
//	dateTime.tm_min = 	BcdToDec(get_time[1]);
//	dateTime.tm_hour = 	BcdToDec(get_time[2]);
//	dateTime.tm_mday = 	BcdToDec(get_time[4]);
//	dateTime.tm_mon = 	BcdToDec(get_time[5])-1;
//	dateTime.tm_year = 	BcdToDec(get_time[6])+100;

  ds3231_Set_Time(ds3231time.seconds, ds3231time.minutes, ds3231time.hours, ds3231time.dayofweek, ds3231time.dayofmonth, ds3231time.month, ds3231time.year);
  ds3231_Get_Time(&ds3231time);

  dateTime.tm_sec  = ds3231time.seconds;
  dateTime.tm_min  = ds3231time.minutes;
  dateTime.tm_hour = ds3231time.hours;
  dateTime.tm_mday = ds3231time.dayofmonth;
  dateTime.tm_mon  = ds3231time.month - 1;
  dateTime.tm_year = ds3231time.year + 100; /* Year - 1900 */

  strftime(buff, sizeof buff, "%A %c", &dateTime);

#if PRINTF_APP_RTC
  sprintf(string, "[app_rtc] [rtos_rtc_thread] Date & Time from RTC (DS3231): %s\n",buff);
  xQueueSend(pPrintQueue, string, 0);
#endif

  /* Get the unix timestamp number of seconds elapsed  since 1st January 1970 */
  epoch = mktime(&dateTime);
//	printf("Epoch: %ld\n", epoch);
  epochBootTime = (epoch-(HAL_GetTick()/1000));
//	printf("Boot time happened at: %d\n", (unsigned int) (epoch-(HAL_GetTick()/1000)));
  bootTime = epoch - epochBootTime;
//	printf("It took: %u secs\n", (unsigned int)bootTime);
  epochtime_ms = epoch*1000 + HAL_GetTick();

  if (pEpoch64)
  {
	*pEpoch64 = epochtime_ms;
  }
  incEnable = 1;
  osThreadTerminate(NULL);
}

void rtos_rtc_increment()
{
#if 0
  UBaseType_t uxSavedInterruptStatus;
  static int localCounter = 0;
#endif
  if (pEpoch64 && incEnable)
  {
	epochtime_ms++;
	(*pEpoch64) = epochtime_ms;
  }
#if 0
  localCounter++;
  if (pEpoch64 && incEnable)
  {
	(*pEpoch64) = epochtime_ms;
  }
  if (localCounter == 19)
  {
	//taskENTER_CRITICAL_FROM_ISR();
	//cycleCounter++;
	//taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
	localCounter = 0;
	//app_sync_notify_from_isr(1);
  }
#endif
}

void RTC_Config_Init()
{ /* Find if an RTC is present in the RAW configuration File */
  CONFIG_WAITING_FOR_DECODE(); /* Block on this line until a RAW configuration File is decoded and valid */
  /* New configuration retrieved. */
#if PRINTF_APP_RTC
   xQueueSend(pPrintQueue, "[app_rtc] [RTC_Config_Init] New configuration retrieved.\n", 0);
#endif
  int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_RTC);
  if (n==1)
  { /* Retrieved one RTC instrument */
	n = getInstrumentFromConfig(&decodedConfig.conf, &RtcInstrumentHandler, SETUP_PRM_COMM_METHOD_RTC);
	if (n == 1)
	{ /* Correctly retrieved instrument pointer */
	  pEpoch64 = (uint64_t*) RtcInstrumentHandler->data;
#if PRINTF_APP_RTC
      xQueueSend(pPrintQueue, "[app_rtc] [RTC_Config_Init] RTC instrument pointer correctly set from RAW Configuration File.\n", 0);
#endif
	}
  }
  else
  { /* No RTC instrument found, or too many RTC's there */
#if PRINTF_APP_RTC
    xQueueSend(pPrintQueue, "[app_rtc] [RTC_Config_Init] No RTC instrument found (or too many) in RAW Configuration File.\n", 0);
#endif
  }
}

int RTC_Config_Still_Valid(void)
{
  if (decodedConfig.state != CONF_CORRECT)
  {
	return 0;
  }
  else
  {
	return 1;
  }
}

void app_rtc_set_unix_epoch_ms(uint64_t epoch_AD_ms)
{
  taskENTER_CRITICAL();
  {
	epochtime_ms = epoch_AD_ms;
  }
  taskEXIT_CRITICAL();

}
void app_rtc_get_unix_epoch_ms(uint64_t * dest)
{
  taskENTER_CRITICAL();
  {
	*dest = epochtime_ms;
  }
  taskEXIT_CRITICAL();
}

uint64_t app_rtc_get_unix_epoch(void)
{
  uint64_t epoch;
  taskENTER_CRITICAL();
  {
	epoch = epochtime_ms/1000;
  }
  taskEXIT_CRITICAL();
  return epoch;
}

uint32_t app_rtc_get_cycle_counter()
{
  uint32_t cCounter;
  taskENTER_CRITICAL();
  {
	cCounter = cycleCounter;
  }
  taskEXIT_CRITICAL();
  return cCounter;
}

void app_rtc_set_cycle_counter(uint32_t cc)
{
  taskENTER_CRITICAL();
  {
	cycleCounter = cc;
  }
  taskEXIT_CRITICAL();
}

void app_rtc_calculate_cycle_counter_from_reference(uint64_t reference)
{
  taskENTER_CRITICAL();
  {
	uint64_t tmp = epochtime_ms - reference;
	cycleCounter = tmp/20;
  }
  taskEXIT_CRITICAL();
//	printf("[APP_RTC] Cycle counter is now : %d\n", cycleCounter);
}

void app_rtc_inc_cycle_counter(void)
{
  cycleCounter++;
}

unsigned int app_rtc_get_rtcUnsyncState(void)
{
  return rtcUnsynchronized;
}

void app_rtc_set_rtUnsyncState(unsigned int state)
{
  if (state > 1)
  {
	state = 1;
  }
  rtcUnsynchronized = state;
}

void app_rtc_print_RTCdateTime(void)
{
  char buff[70];
  ds3231_time_t getds3231time;
  struct tm RTCdateTime;
  memset(&RTCdateTime, 0, sizeof(RTCdateTime));

  ds3231_Get_Time(&getds3231time);

  RTCdateTime.tm_sec  = getds3231time.seconds;
  RTCdateTime.tm_min  = getds3231time.minutes;
  RTCdateTime.tm_hour = getds3231time.hours;
  RTCdateTime.tm_wday = getds3231time.dayofweek - 1; // tm structure dow: Sunday = 0,       DS3231 dow: Sunday = 1
  RTCdateTime.tm_mday = getds3231time.dayofmonth;
  RTCdateTime.tm_mon  = getds3231time.month - 1;     // tm structure month is from 0 to 11, DS3231 month is from 1 to 12
  RTCdateTime.tm_year = getds3231time.year + 100;    // tm structure year is year - 1900,   DS3231 year is from 0-99

  strftime(buff, sizeof buff, "%A %c", &RTCdateTime);

#if PRINTF_APP_RTC
  sprintf(string, "[app_rtc] [app_rtc_print_RTCdateTime] RTC Date & Time synchronized from Android Device: %s\n",buff);
  xQueueSend(pPrintQueue, string, 0);
#endif

}

void compare_RTCunixtime_Epoch(void)
{
  uint64_t epochNow_Ms = 0;
  app_rtc_get_unix_epoch_ms(&epochNow_Ms); 								/* Get the epoch unix time from the RTC */
  ds3231_time_t getds3231time;
  ds3231_Get_Time(&getds3231time);

#if PRINTF_APP_RTC
  sprintf(string, "[app_rtc] [compare_RTCunixtime_Epoch] RTC: %08X%08X, epochtime_ms: %08X%08X.\n",
		  (uint32_t)(getds3231time.unixtime>>32), (uint32_t)getds3231time.unixtime, (uint32_t)(epochNow_Ms>>32), (uint32_t)epochNow_Ms);
  xQueueSend(pPrintQueue, string, 0);
#endif
}


