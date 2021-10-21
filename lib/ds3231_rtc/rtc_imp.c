/**
 * @file rtc_imp.c
 * @brief RTC Implementation
 * @author Alexis.C, Ali.O - Yncrea HdF
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */
#include "rtc.h"
#include "rtc_imp.h"
#include "stm32h7xx_hal_rcc.h"
#include "ds3231.h"

extern RTC_HandleTypeDef hrtc;


void setTime(char hour, char min,   char sec)
{
    RTC_TimeTypeDef  stimestructure;

      stimestructure.Hours = hour;
      stimestructure.Minutes = min;
      stimestructure.Seconds = sec;
      stimestructure.TimeFormat = RTC_HOURFORMAT12_AM;
      stimestructure.DayLightSaving = RTC_DAYLIGHTSAVING_NONE ;
      stimestructure.StoreOperation = RTC_STOREOPERATION_RESET;

      if(HAL_RTC_SetTime(&hrtc,&stimestructure,FORMAT_BCD) != HAL_OK)
      {
        /* Initialization Error */
        Error_Handler();
      }
}

void setDate(char year, char month, char day)
{
    RTC_DateTypeDef  sdatestructure;

    sdatestructure.Year = year;
    sdatestructure.Month = month;
    sdatestructure.Date = day;
    sdatestructure.WeekDay = RTC_WEEKDAY_TUESDAY;

    if(HAL_RTC_SetDate(&hrtc,&sdatestructure,FORMAT_BCD) != HAL_OK)
    {
      /* Initialization Error */
      Error_Handler();
    }

}

void getTime(char * hour, char * min,   char * sec)
{
#if 0
    RTC_TimeTypeDef stimestructureget;

    memset(&stimestructureget, 0, sizeof(stimestructureget));

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&hrtc, &stimestructureget, FORMAT_BIN);
#else
    uint32_t tmpreg = 0;

    /* Get the TR register */
    tmpreg = (uint32_t)(hrtc.Instance->TR & RTC_TR_RESERVED_MASK);

    /* Fill the structure fields with the read parameters */
    *hour = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
    *min =  (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
    *sec =  (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));

   /* Convert the time structure parameters to Binary format */
   *hour  = (uint8_t)RTC_Bcd2ToByte(*hour);
   *min = (uint8_t)RTC_Bcd2ToByte(*min);
   *sec = (uint8_t)RTC_Bcd2ToByte(*sec);

#endif
}

void getTimeWithSubs(char * hour, char * min, char * sec, unsigned int *subsecs, unsigned int *fraction)
{
#if 0
    RTC_TimeTypeDef stimestructureget;

    memset(&stimestructureget, 0, sizeof(stimestructureget));

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&hrtc, &stimestructureget, FORMAT_BIN);
#else
    /* Get subseconds structure field from the corresponding register*/
   *subsecs = (uint32_t)(hrtc.Instance->SSR);

   /* Get SecondFraction structure field from the corresponding register field*/
   *fraction = (uint32_t)(hrtc.Instance->PRER & RTC_PRER_PREDIV_S);

   uint32_t tmpreg = 0;

   /* Get the TR register */
   tmpreg = (uint32_t)(hrtc.Instance->TR & RTC_TR_RESERVED_MASK);

    /* Fill the structure fields with the read parameters */
   *hour = (uint8_t)((tmpreg & (RTC_TR_HT | RTC_TR_HU)) >> 16);
   *min  = (uint8_t)((tmpreg & (RTC_TR_MNT | RTC_TR_MNU)) >>8);
   *sec  = (uint8_t)(tmpreg & (RTC_TR_ST | RTC_TR_SU));

   /* Convert the time structure parameters to Binary format */
   *hour = (uint8_t)RTC_Bcd2ToByte(*hour);
   *min  = (uint8_t)RTC_Bcd2ToByte(*min);
   *sec  = (uint8_t)RTC_Bcd2ToByte(*sec);

#endif
}


void getDate(char * year, char * month, char * day)
{
    RTC_DateTypeDef sdatestructureget;
    /* Get the RTC current Date */
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, FORMAT_BIN);

    *year = sdatestructureget.Year;
    *month= sdatestructureget.Month;
    *day  = sdatestructureget.Date;
}

void showTime(char* showtime, unsigned int *cx)
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;

    memset(&sdatestructureget, 0, sizeof(sdatestructureget));
    memset(&stimestructureget, 0, sizeof(stimestructureget));

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&hrtc, &stimestructureget, FORMAT_BIN);
    /* Get the RTC current Date */
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, FORMAT_BIN);
    /* Display time Format : hh:mm:ss */
    *cx = sprintf((char*)showtime,"20%d-%d-%d/%02d:%02d:%02d\n",sdatestructureget.Year, sdatestructureget.Month, sdatestructureget.Date,
    													stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
}

void printTime()
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;

    memset(&sdatestructureget, 0, sizeof(sdatestructureget));
    memset(&stimestructureget, 0, sizeof(stimestructureget));

    /* Get the RTC current Time */
    HAL_RTC_GetTime(&hrtc, &stimestructureget, FORMAT_BIN);
    /* Get the RTC current Date */
    HAL_RTC_GetDate(&hrtc, &sdatestructureget, FORMAT_BIN);
    /* Display time Format : hh:mm:ss */
    //printf("20%d-%d-%d/%02d:%02d:%02d\n",sdatestructureget.Year, sdatestructureget.Month, sdatestructureget.Date,
    //													stimestructureget.Hours, stimestructureget.Minutes, stimestructureget.Seconds);
}

/* Function to retrieve date and time from a DS3231 on I2C bus and synchronize the internal RTC */
void setInternalRtc()
{
    RTC_DateTypeDef sdatestructureget;
    RTC_TimeTypeDef stimestructureget;

    memset(&sdatestructureget, 0, sizeof(sdatestructureget));
    memset(&stimestructureget, 0, sizeof(stimestructureget));

    /* Retreive date and time from the external RTCC */
    ds3231_get_datetime(&stimestructureget, &sdatestructureget);

    if(HAL_RTC_SetDate(&hrtc,&sdatestructureget,RTC_FORMAT_BIN) != HAL_OK)
    {
      /* Initialization Error */
      Error_Handler();
    }
    if(HAL_RTC_SetTime(&hrtc,&stimestructureget,RTC_FORMAT_BIN) != HAL_OK)
    {
      /* Initialization Error */
      Error_Handler();
    }
}

/*void testRtc()
{
    for (unsigned int i = 0; i<30; i++)
    {
        ds3231_refresh_time();
        char b[128];
        b[0] = NULL;
        unsigned int cx = 0;
        showTime(b, &cx);
        HAL_UART_Transmit(&huart3, b, cx, 0xfff);
        osDelay(1000);
    }
}*/


