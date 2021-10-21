/**
 * @file app_sync.c
 * @brief Synchronization between storage and streaming
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 */

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "config/config_op.h"

#include "app_streamer.h"
#include "app_storage.h"
#include "app_rtc.h"
#include "app_sync.h"
#include "usart.h"

#include "gpio.h"
#include "main.h"
#include "common.h"

#define PRINT_APP_SYNC_DBG_MSG						1


void syncThread(const void * params);
static osThreadId syncThreadHandler;

extern char string[];

void initSyncThread()
{
  osThreadDef(syncThread, syncThread, osPriorityNormal, 0, 256); /* SyncThread definition  */
  syncThreadHandler = osThreadCreate (osThread(syncThread), NULL);    /* Start SyncThread  */
}

void syncThread(const void * params)
{
  uint32_t ulInterruptStatus;
  uint64_t epochNow_Ms = 0;
  while (decodedConfig.state != CONF_CORRECT)
  {  /* Loop until the configuration is decoded */
	osDelay(20);
  }
  xTaskNotifyStateClear(NULL); /* Clear the notification before entering the while loop */
  for(;;)
  {
	/* Block until we get the notification from the timer interruption */
	if (xTaskNotifyWait( 0x00,                         /* Don't clear any bits on entry. */
				         0xffffffff,                   /* Clear all bits on exit. (long max) */
				         &ulInterruptStatus,           /* Receives the notification value. */
				         portMAX_DELAY ) == pdTRUE)    /* Block task undefinetely */
	{ /* Each 20ms, snapshot (copy) the configuration and copy decodedConfig in snapshotConfig */
	  decodedConfig.get(); 												// Get the mutex of the decoded config
	  decodedConfig.conf.cycleCounter = app_rtc_get_cycle_counter(); 	// Get the cycle counter and store it in the decoded config
	  config_copy(&snapshotconf, &decodedConfig.conf); 					// Make a copy of the decoded config
	  decodedConfig.release(); 											// Release the mutex of the decoded config
	  app_streamer_notify(APP_STREAMER_NOTIF_CYCLE_COUNTER);  			// Notify the streamer thread
	  app_storage_notify(APP_STORAGE_NOTIF_CYCLE_COUNTER);				// Notify the storage thread
	}
  }
}

void app_sync_notify_from_isr(uint32_t notValue)
{
  if (osKernelRunning())
  {
	BaseType_t xHigherPriorityTaskWoken;
	BaseType_t res;
	xHigherPriorityTaskWoken = pdFALSE;
	if (syncThreadHandler != NULL)
	{
	  res = xTaskNotifyFromISR( syncThreadHandler, notValue, eSetBits, &xHigherPriorityTaskWoken );
	  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
	}
  }
}
