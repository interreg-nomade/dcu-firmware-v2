/**
 * @file app_tablet_com.c

 * @brief Application for the mainboard-tablet communication
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 *
 * Adapted for use in NOMADe project
 *
 * Sarah Goossens
 * August 2020
 *
 */

/* RTOS Header part */
#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

/* APP header part */
#include "app_tablet_com.h"
#include "app_rtc.h"
#include "../lib/ds3231_rtc/ds3231.h"
#include "app_streamer.h"
#include "app_storage.h"

/* Communication protocol Header part */
#include "tablet_com_protocol/frames.h"
#include "tablet_com_protocol/parser.h"
#include "tablet_com_protocol/fc_tx.h"
#include "tablet_com_protocol/fc_rx.h"
#include "tablet_com_protocol/fc_frames.h" //#include "../lib/tablet_com_protocol/fc_frames.h"

/* SD interface */
#include "interface_sd.h"

/* Measurement list Header part */
#include "stomeas_lib/storage_measurements_list.h"

/* Queue header part */
#include "queues/streamer_service/streamer_service_queue.h"
#include "queues/notification/notification_service_queue.h"
#include "queues/measurements/measurements_queue.h"
#include "queues/android_device_com/android_com_queue.h"

/* Data structure header part*/
#include "data/structures.h"
#include "data_op/op.h"
#include "fifo/fifo.h"

/* Hardware interface header part */
#include "board.h"
#include "usart.h"
#include "pUART.h" /* Contains UART ressource (Mutex/Sempahore) */

/* Logs header part */
//#include "logs/devlogs.h"

#include "common.h"
#include "imu_com.h"

#define PRINT_APP_TABLET_COM_DBG_MSG 1
#if PRINT_APP_TABLET_COM_DBG_MSG
#define PRINT_APP_TABLET_RECEIVE_WD				0
#define PRINT_APP_TABLET_COM_SERVICE_ID			0
#define PRINT_APP_TABLET_DEVICE_ADDRESS			1
#define PRINT_APP_TABLET_SETUPID_VERSION		0
#define PRINT_APP_TABLET_RECEIVE_ACK			0
#define PRINT_APP_TABLET_RECEIVE_NACK			0
#endif

#define PRINTF_RTOS_UPLINK 1
#define PRINTF_RTOS_UPLINK_DEBUG2 1

#define DEFAULT_MAINBOARD_ADDRESS           0x81
#define USB_TABLET_COM_FLOW_CONTROL_TIMEOUT 100
#define USB_TABLET_COM_WATCHDOG_TIMEOUT     500
#define USB_AUTOMATIC_COMMUNICATION 		  1

/* ***** Watchdog ***** */
static TimerHandle_t xWatchdogTimerHandle;
static unsigned int tLastWatchdogMsg = 0;
void watchdogUsbRxCallback(void);
void watchdogSetTime(uint32_t watchdogValueInms);
uint32_t watchdogGetTime(void);
void watchdog_timer_init(void);
void watchdogTimerCallback(TimerHandle_t xTimer);
/* ******************* */

/* Thread handler */
static osThreadId cplRxTaskHandle;
/* Thread function */
void upLinksManagerThread(const void *params);
/* ***** Flow control transmission ***** */
static fc_tx_handler_t fcTxHandler;
/* ***** Flow control reception ******** */
static fc_rx_handler_t flowControlRx;
/* ************************************* */

/* Cloud link protocol variables */
static cpl_msg_t CplRxMsg;
/* The first available address is 1 */
static unsigned int  firstAvailableAddress = 1;
upLinkStatus_t androidTabLinkHandler;

/* Private functions */
void sendCurrentSetupID(unsigned int id, unsigned int version, unsigned int ad);
void sendRequestAnswer(unsigned int id, unsigned int ad);
void streamAnswer(unsigned int id, unsigned int ad);
void sendStreamRequestAnswer(unsigned int id, unsigned int ad);

//void cp_ServiceHandler(cpl_msg_t * pMsg);
void cpl_ServiceHandler(cpl_msg_t                     * pMsg,
		                fc_tx_handler_t               * pfcTxHandler,
						fc_rx_handler_t               * pflowControlRx,
		                measurementsServiceQueueMsg_t * serviceQueueMsg);

int refreshTxHandlerDataJson(struct fc_tx_handler_t * self);
void cplAssignAddressID(unsigned char currentAddress, /* which is temporary */
		                unsigned char addressAssigned,
		                unsigned char sourceAddress);
void tablet_com_send_watchdog_msg(char ad);
void tablet_com_enabling_streaming(unsigned char nPacket, unsigned short nBytes);
void tablet_com_disable_streaming(void);
void tablet_com_stop_measurement(uint64_t stopTime);
void tablet_com_stop_measurement_from_meas_list_callback(unsigned int measurementId);
void tablet_com_start_measurement_from_meas_list_callback(unsigned int measurementId);

/**** Flow control prototype function ****/
void flowInitACK(unsigned int npackets, unsigned char sourceAd,unsigned char destAd, unsigned int dsap);
void flowACK(unsigned int npackets, unsigned char destination, unsigned char source );
void flowNACK(unsigned int npackets, unsigned char destination, unsigned char source );
/**** Flow control receive callback function ****/
void fc_json_data_rx_cplt_callback(void);
void fc_json_data_rx_callback(unsigned char * buffer, unsigned int length);
void fc_raw_data_rx_callback(unsigned char * buffer, unsigned int length);
void fc_raw_data_rx_cplt_callback(void);
void fc_measurement_list_rx_callback(unsigned char * buffer, unsigned int length);
void fc_measurement_list_data_rx_cplt_callback(void);


// to ensure that we inform app_BT1 to start streaming to the measurement queue
streamerServiceQueueMsg_t streamMsgEnabled;



extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];
extern char string[];
extern QueueHandle_t pPrintQueue;

void cpl_init_rx_task(void)
{
  osDelay(100);
  cpl_init(); 																	/* Init decoder */
  memset(&CplRxMsg, 0, sizeof(cpl_msg_t));
  memset(&flowControlRx, 0, sizeof(fc_rx_handler_t));
  sprintf(string, "cpl_init_rx_task done. \n");
  HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
  osThreadDef(upLinksManager, upLinksManagerThread, osPriorityRealtime, 0, 4096);   /* Create upLinksManagerThread, was priority normal, but then issue with stopping a measurement */
  cplRxTaskHandle = osThreadCreate(osThread(upLinksManager), NULL);				/* Start upLinksManagerThread */
  watchdog_timer_init(); 														/* Init watchdog timer and start it */
}

void upLinksManagerThread(const void *params)
{
  uint64_t epochNow_Ms;
  uint32_t ulNotificationValue = 0; 				/* Used for notification between the task and the UART receive interrupt */
  TickType_t xMaxBlockTime = pdMS_TO_TICKS(400); 	/* The maximum block time state of the task before receive a notification */
  measurementsServiceQueueMsg_t serviceQueueMsg; 	/* Used receive information from the storage task */
  bool startMeasurementFromList = false; 			/* Flag used to indicate that a measurement from the list has been started */
  CPL_OP_RESULT opRes =  CPL_NO_MSG; 				/* Used to store the result of decoded message */
  memset(&serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t)); /* Put the message in a known state */
  androidTabLinkHandler.mainboardAddress = DEFAULT_MAINBOARD_ADDRESS; /* Assign the default address of the mainboard and the android device */
  for(;;)
  {
//#if PRINTF_RTOS_UPLINK
//  if (serviceQueueMsg.status != measurement_StatusOriginIDLE)
//	{
//	  sprintf(string, "[APP_TABLET_COM] [upLinksManagerThread] serviceQueueMsg.status = 0x%0X.\n", (unsigned int) serviceQueueMsg.status);
//	  xQueueSend(pPrintQueue, string, 0);
//	}
//#endif
    switch(serviceQueueMsg.status)
	{
	  /* If the measurement is in IDLE state check if we receive a message from the storage task */
	  case measurement_StatusOriginIDLE: /* No measurement request was launched */
	  {
		if(android_com_receive(&serviceQueueMsg)) /* Get a message from the Queue */
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] New message received from the service queue while idle measurement status.\n", 0);
#endif
		  if(serviceQueueMsg.status == measurement_StatusOriginMeasurementList) /* The notification comes only from the storage task */
		  {
			if(serviceQueueMsg.action == measurement_StartMeasurement) /* The measurement from the list has to be launched */
			{
			  if(tablet_com_is_online())
			  {
#if PRINTF_RTOS_UPLINK
				sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] Android device is online and a measurement from list has to be launched MEAS ID: %d, start time: %llu.\n",
							(unsigned int) HAL_GetTick(), serviceQueueMsg.measurementId, (long long unsigned)serviceQueueMsg.startTime);
				xQueueSend(pPrintQueue, string, 0);
#endif
				tablet_com_start_measurement_from_meas_list_callback(serviceQueueMsg.measurementId); /* Send Start Measurement to android device */
				startMeasurementFromList = true; /* Raise Start Measurement From List Flag */
			  }
			}
			else if(serviceQueueMsg.action == measurement_StopMeasurement)
			{
			  /* The measurement from the list has to stop. For the moment we are never in this case */
			}
		  }
		}
		break;
	  }
	  case measurement_StatusOriginAndroidDevice: /* Measurement requested by android device is in progress */
	  {
		if(serviceQueueMsg.action == measurement_StartMeasurement)
		{ /* Measurement is in progress */
		  if(android_com_receive(&serviceQueueMsg))
		  {
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] New message received from the service queue while measurement in progress.\n", 0);
#endif
			if(serviceQueueMsg.status == measurement_StatusOriginMeasurementList)
			{
			  if(serviceQueueMsg.action == measurement_StartMeasurement)
			  {
				/* The measurement from the list has to be launched but a measurement from the android device is in progress */
				/* Send stop measurement to the android device before launching the measurement from the measurement list */
				app_rtc_get_unix_epoch_ms(&epochNow_Ms); /* Get the epoch unix time from the RTC */
#if PRINTF_RTOS_UPLINK
				xQueueSend(pPrintQueue, "[APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] Measurement from list has to be started but first we need to stop measurement initiated by android device.\n", 0);
#endif
				tablet_com_stop_measurement(epochNow_Ms); /* Send stop measurement to the android device */
				/* Now waiting for ACK from android device before start measurement from list can be started */
			  }
			  else if(serviceQueueMsg.action == measurement_StopMeasurement)
			  {
				/* If a measurement from the android device has been launched and the measurement from the list has to be stopped do nothing */
				//TODO : Get back the value of the serviceQueueMsg
			  }
			}
		  }
		}
		break;
	  }
	  case measurement_StatusOriginMeasurementList: /* Measurement launched from the measurement list */
	  {
		if(android_com_receive(&serviceQueueMsg))
		{
		  if(serviceQueueMsg.action == measurement_StartMeasurement)
		  {
		    /* Normally we will never reach here */
		  }
		  else if(serviceQueueMsg.action == measurement_StopMeasurement)
		  {
			if(tablet_com_is_online()&& startMeasurementFromList) /* We need to stop the measurement */
			{
#if PRINTF_RTOS_UPLINK
			  sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] Android device is online and a measurement from list has to be stopped MEAS ID: %d, start time: %llu.\n",
									(unsigned int) HAL_GetTick(), serviceQueueMsg.measurementId, (long long unsigned)serviceQueueMsg.stopTime);
			  xQueueSend(pPrintQueue, string, 0);
#endif
			  tablet_com_stop_measurement_from_meas_list_callback(serviceQueueMsg.measurementId); /* Send stop measurement form list to the android device */
			  startMeasurementFromList = false; /* Clear the flag */
			}
		  }
		}
		else
		{
		  if(serviceQueueMsg.action == measurement_StartMeasurement)
		  {
			if(!startMeasurementFromList)
			{
			  if(tablet_com_is_online())
			  {
#if PRINTF_RTOS_UPLINK
				sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] [serviceQueueMsg] Android device is online and a measurement from list has to be launched MEAS ID: %d, start time: %llu.\n",
							(unsigned int) HAL_GetTick(), serviceQueueMsg.measurementId, (long long unsigned)serviceQueueMsg.startTime);
				xQueueSend(pPrintQueue, string, 0);
#endif
				tablet_com_start_measurement_from_meas_list_callback(serviceQueueMsg.measurementId); /* Send Start Measurement to the android device */
				startMeasurementFromList = true; /* Raise Start Measurement From List Flag */
			  }
			}
			else if(serviceQueueMsg.action == measurement_StopMeasurement)
			{
			    /* Normally we will never reach here */
			}
		  }
		}
		break;
	  }
	  default:
	  {
		break;
	  }
	}
	if(xTaskNotifyWait( 0x00,               	/* Don't clear any bits on entry. */
						0xffffffff,          	/* Clear all bits on exit. (long max) */
						&ulNotificationValue, 	/* Receives the notification value. */
						xMaxBlockTime ) == pdTRUE) /* The maximum block time state of the task before receive a notification */
	{
	  opRes = cpl_prot_decoder(&CplRxMsg, (uint32_t) xTaskGetTickCount()); /* Launch the decoder*/
	  if (opRes == CPL_CORRECT_FRAME) /* If a correct frame is received Handle the related action */
	  {
#if PRINTF_RTOS_UPLINK_DEBUG2
//	      if (CplRxMsg.destinationService != 0x00)
//	      {
//	        sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] Received telegram 0x68%02X%02X68%02X%02X%02X%02X%02X",
//	                		        (unsigned int) HAL_GetTick(),
//									CplRxMsg.lenght,
//									CplRxMsg.lenght,
//									CplRxMsg.destinationAddress,
//									CplRxMsg.sourceAddress,
//									CplRxMsg.FC,
//									CplRxMsg.destinationService,
//									CplRxMsg.sourceService);
//		    xQueueSend(pPrintQueue, string, 0);
//	        sprintf(string, " Payload: ");
//	        char DUString[3];
//	        for (unsigned int i = 0; i<CplRxMsg.payloadLength; i++)
//	        {
//	          sprintf(DUString, "%02X", CplRxMsg.DU[i]);
//	          strcat(string, DUString);
//	        }
//          strcat(string, ".\n");
////	    sprintf(string, "%02X16.\n",CplRxMsg.FCS);
//	        xQueueSend(pPrintQueue, string, 0);
////	    //sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] Received a CPL Packet with ID = 0x%04x and function code 0x%04x\n",(unsigned int) HAL_GetTick(), CplRxMsg.SERVICE, CplRxMsg.FC);
//	        sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] cpl_ServiceHandler(CplRxMsg, fcTxHandler = 0x%0X, flowControlRx = 0x%0X, serviceQueueMsg = 0x%0X)\n",
//	                		            (unsigned int) HAL_GetTick(),
//										(unsigned int) fcTxHandler.bufferElems,
//										(unsigned int) flowControlRx.Enabled,
//										(unsigned int) serviceQueueMsg.measurementId);
//		    xQueueSend(pPrintQueue, string, 0);
//	      }
#endif
	    cpl_ServiceHandler(&CplRxMsg, &fcTxHandler, &flowControlRx, &serviceQueueMsg);
	  }
	  else if (opRes == CPL_WRONG_CRC)
	  {
#if PRINTF_RTOS_UPLINK
        sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] CPL_WRONG_CRC for packet number %d\n", (unsigned int) HAL_GetTick(), CplRxMsg.packetNumber);
		xQueueSend(pPrintQueue, string, 0);
#endif
	    if (CplRxMsg.FC == FLOW_CONTROL_FUNCTION_CODE_DATA)
	    {
	      /* TODO : Wrong CRC in the received message send a NACK for this packet */
#if PRINTF_RTOS_UPLINK
          sprintf(string, "%u [APP_TABLET_COM] [upLinksManagerThread] [opRes = CPL_WRONG_CRC] [FC = FLOW_CONTROL_FUNCTION_CODE_DATA] Send a NACK for packet number %d\n", (unsigned int) HAL_GetTick(), CplRxMsg.packetNumber);
		  xQueueSend(pPrintQueue, string, 0);
#endif
	    }
	  }
	}
  }
}

void app_tablet_com_notify_from_isr(uint32_t notValue)
{
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  if (cplRxTaskHandle != NULL)
  {
	xTaskNotifyFromISR( cplRxTaskHandle, notValue, eSetBits, &xHigherPriorityTaskWoken );
  }
  portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void app_tablet_com_notify(uint32_t notValue)
{
  if (cplRxTaskHandle != NULL)
  {
	xTaskNotify( cplRxTaskHandle, notValue, eSetBits);
  }
}

/* Function handling classic services */
void cpl_ServiceHandler(cpl_msg_t * pMsg, fc_tx_handler_t * pfcTxHandler, fc_rx_handler_t * pflowControlRx, measurementsServiceQueueMsg_t * serviceQueueMsg)
{
  static unsigned int FC_PacketsExpected	= 0;
  static unsigned int FC_ACK_Every 	   		= 0;
//static unsigned int FC_TimeoutValue    	= 0;
  switch (pMsg->FC)
  {
    case FLOW_CONTROL_FUNCTION_CODE_NOFLOW:
	{
#if PRINTF_RTOS_UPLINK
//	  if (CplRxMsg.destinationService != 0x00)
//	  {
//	    sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] Received service 0x%04X\n",(unsigned int) HAL_GetTick(), pMsg->SERVICE);
//	    xQueueSend(pPrintQueue, string, 0);
//	  }
#endif
	  switch(pMsg->SERVICE)
	  {
		case SERVICE_SOFTWARE_WATCHDOG: /* Received Watchdog Message */
		{
#if PRINTF_RTOS_UPLINK
//		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received Watchdog Message.\n", 0);
#endif
		  watchdogUsbRxCallback(); 												/* Update the watchdog timer */
		  tablet_com_send_watchdog_msg(androidTabLinkHandler.tabletAddress); 	/* Send the watchdog message */
		  break;
		}
		case SERVICE_REQUEST_CONNECTION_CHECK: /* A tablet connects and needs an address */
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received Service Request ASK Address.\n", 0);
#endif
		  androidTabLinkHandler.tabletAddress = ++firstAvailableAddress;
		  if ((pMsg->destinationAddress == 0xFF) && (pMsg->sourceAddress == 0xFE)) /* Check the DA and SA field of the message */
		  {
#if PRINTF_RTOS_UPLINK
		    sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] Assiging ID = 0x%02x\n",androidTabLinkHandler.tabletAddress);
			xQueueSend(pPrintQueue, string, 0);
			sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] pMsg->destinationAddress == 0x%02x\n", pMsg->destinationAddress);
			xQueueSend(pPrintQueue, string, 0);
			sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] pMsg->sourceAddress == 0x%02x\n", pMsg->sourceAddress);
			xQueueSend(pPrintQueue, string, 0);
#endif
			cplAssignAddressID(pMsg->sourceAddress, androidTabLinkHandler.tabletAddress, androidTabLinkHandler.mainboardAddress); /* Send address to the ANDROID Device */
			sendCurrentSetupID( getConfigId(), getConfigVersion(), androidTabLinkHandler.tabletAddress ); /* Send current set up ID and version */
#if USB_AUTOMATIC_COMMUNICATION == 1
			androidTabLinkHandler.isOnline = 1;
#endif
		  }
		  break;
		}
		case SERVICE_RESPONSE_CONNECTION_CHECK:
		{
		  break;
		}
		case SERVICE_REQUEST_SETUP_ID: /* Receive a message asking for the actual SETUP */
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received Service Request Setup ID.\n", 0);
#endif
		  sendCurrentSetupID( getConfigId(), getConfigVersion(), androidTabLinkHandler.tabletAddress ); /* Send current set up ID and version */
		  break;
		}
		case SET_RTC_TIME: /* Receive the time in epoch_64 ms format */
		{
//#if PRINTF_RTOS_UPLINK
//		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received GMT date and time from Android device.\n", 0);
//		  sprintf(string,"[APP_TABLET_COM] [cp_ServiceHandler] epoch_64 time: ");
//  	      char DUString[4];
//      	  for (unsigned int i = 0; i < 8; i++)
//  	      {
//  	    	  if (strlen(string) < 147)
//  	    	  {
//      	    	sprintf(DUString, "%02X ",pMsg->DU[i]);
//      	    	strcat(string, DUString);
//  	    	  }
//  	      }
//   	      strcat(string,"\n");
//  	      xQueueSend(pPrintQueue, string, 0);
//#endif
  	      uint64_t epoch_AD_ms = pMsg->DU[0];
      	  for (unsigned int i = 1; i < 8; i++)
  	      {
      		epoch_AD_ms = (epoch_AD_ms << 8) | pMsg->DU[i];
  	      }
//#if PRINTF_RTOS_UPLINK
//  	      sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] epoch_AD_ms from Android Device: %08X%08X.\n",(uint32_t)(epoch_AD_ms>>32),(uint32_t)epoch_AD_ms);
//  	      xQueueSend(pPrintQueue, string, 0);
//#endif
      	  //epoch_AD_ms += 60*60*2*1000; //  GMT+02:00 for DST
      	  app_rtc_set_unix_epoch_ms(epoch_AD_ms);
      	  uint64_t epoch_AD_s = epoch_AD_ms/1000;
  	      struct tm dateTime;
  	      char buff[70];
  	      memset(&dateTime, 0, sizeof(dateTime));
  	      dateTime = *localtime(&epoch_AD_s);
  	      strftime(buff, sizeof buff, "%A %c", &dateTime);

#if PRINTF_RTOS_UPLINK
  	      sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] Date & Time received from Android Device: %s\n",buff);
  	      xQueueSend(pPrintQueue, string, 0);
#endif
  	      ds3231_time_t ds3231time;
  	      ds3231time.seconds    = dateTime.tm_sec;
  	      ds3231time.minutes    = dateTime.tm_min;
  	      ds3231time.hours      = dateTime.tm_hour;
  	      ds3231time.dayofweek  = dateTime.tm_wday + 1;   // tm structure dow: Sunday = 0, DS3231 dow: Sunday = 1
  	      ds3231time.dayofmonth = dateTime.tm_mday;
  	      ds3231time.month      = dateTime.tm_mon + 1;    // tm structure month is from 0 to 11, DS3231 month is from 1 to 12
  	      ds3231time.year       = dateTime.tm_year - 100; // tm structure year is year - 1900, DS3231 year is from 0-99

  	      ds3231_Set_Time(ds3231time.seconds, ds3231time.minutes, ds3231time.hours, ds3231time.dayofweek, ds3231time.dayofmonth, ds3231time.month, ds3231time.year);

  	      app_rtc_print_RTCdateTime();
  	      compare_RTCunixtime_Epoch();

//  	      ds3231_time_t getds3231time;
//  	      struct tm RTCdateTime;
//  	      memset(&RTCdateTime, 0, sizeof(RTCdateTime));
//
//  	      ds3231_Get_Time(&getds3231time);
//
//  	      RTCdateTime.tm_sec  = getds3231time.seconds;
//  	      RTCdateTime.tm_min  = getds3231time.minutes;
//  	      RTCdateTime.tm_hour = getds3231time.hours;
//  	      RTCdateTime.tm_wday = getds3231time.dayofweek - 1;
//  	      RTCdateTime.tm_mday = getds3231time.dayofmonth;
//  	      RTCdateTime.tm_mon  = getds3231time.month - 1;
//  	      RTCdateTime.tm_year = getds3231time.year + 100;
//
//  	      strftime(buff, sizeof buff, "%A %c", &RTCdateTime);
//
//#if PRINTF_RTOS_UPLINK
//  	      sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] RTC Date & Time synchronized from Android Device: %s\n",buff);
//  	      xQueueSend(pPrintQueue, string, 0);
//#endif
		  break;
		}
		case SERVICE_RESPONSE_SETUP_ID: /* Receive the ACK for the SETUP message */
		{
		  uint16_t config	= 0x00;
		  uint16_t version	= 0x00;
		  config 			= pMsg->DU[0] << 8 | pMsg->DU[1];
		  version			= pMsg->DU[2] << 8 | pMsg->DU[3];
		  if(config == getConfigId() && version == getConfigVersion())
		  {
#if PRINTF_RTOS_UPLINK
			xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Receive Setup ID and Version ACK from the android device.\n", 0);
#endif
			androidTabLinkHandler.isOnline = 1; /* The android device is online */
		  }
		  break;
		}
		case SERVICE_RESPONSE_NO_SETUP_ID: /* Receive the ACK for the NO SETUP message */
		{
		  if(!pMsg->DU[0] && !pMsg->DU[1] && !pMsg->DU[2] && !pMsg->DU[3])
		  {
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Receive NO Setup ID ACK from the android device.\n", 0);
#endif
			androidTabLinkHandler.isOnline = 1; /* The android device is online */
		  }
		  break;
		}
		case SERVICE_REQUEST_START_DATA_STREAM: /* Receive START DATA stream message */
		{
		  if(serviceQueueMsg->status != measurement_StatusOriginMeasurementList)
		  {
#if PRINTF_RTOS_UPLINK
		    sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] Received request start data stream. Payload size: %d.\n",pMsg->payloadLength);
			xQueueSend(pPrintQueue, string, 0);
#endif
			unsigned char nPacket = 0; /* Hold how many packets should be exchanged every 20 ms */
			unsigned short nBytes = 0; /* Hold how many bytes should be exchanged every 20 ms */
			nPacket = pMsg->DU[0];
			nBytes = (pMsg->DU[1] << 8) | pMsg->DU[2];
		    app_rtc_set_cycle_counter(1); 												/* set cycle counter to 1, moved from line 589 */
			sendStreamRequestAnswer(SERVICE_RESPONSE_START_DATA_STREAM, androidTabLinkHandler.tabletAddress); /* Send answer of streaming request to android device */
// moved to app_init.c
//			IMU_start_measurements_without_sync(&imu_1); // send message to BT1 to start the IMU measurement (temporary solution)
			tablet_com_enabling_streaming(nPacket, nBytes); /* Enable streaming */
		  }
		  else if(serviceQueueMsg->status == measurement_StatusOriginMeasurementList)
		  {
		    if(serviceQueueMsg->action == measurement_StartMeasurement)
			{
			  if(app_storage_active())
			  {
#if PRINTF_RTOS_UPLINK
				xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received request start measurement when a measurement from list is in progress.\n", 0);
#endif
			  }
			}
			else if(serviceQueueMsg->action == measurement_StopMeasurement)
			{
#if PRINTF_RTOS_UPLINK
			  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received request stop measurement when a measurement from list has to be finished.\n", 0);
#endif
			}
		  }
		  break;
		}
		case SERVICE_RESPONSE_PACKET_DATA_STREAM: /* Receive ACK of DATA stream message */
		{
		  watchdogUsbRxCallback(); /* Update the watchdog timer */
		  streamerServiceQueueMsg_t streamMsg; /* Response to ACK of streaming packet from the ANDROID Device */
		  streamMsg.theirCycleCounter = (pMsg->DU[0] << 24) | (pMsg->DU[1] << 16) | (pMsg->DU[2] << 8) | (pMsg->DU[3]); /* Get cycle counter received by android device */
		  if(pMsg->DU[4] == 0xFE)
		  { /* A packet is missing for more than 60 ms received ==> NACK from the android device */
#if PRINTF_RTOS_UPLINK
		    sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] Missing packet for cycle counter: 0x%08X.\n",(unsigned int) HAL_GetTick(), streamMsg.theirCycleCounter);
			xQueueSend(pPrintQueue, string, 0);
#endif
			streamMsg.action =  streamingService_NACKAndroidStream;
		  }
		  else if(pMsg->DU[4] == 0x80)
		  {
//#if PRINTF_RTOS_UPLINK
//		    sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] Received ACK for cycle counter: 0x%08X.\n",(unsigned int) HAL_GetTick(), streamMsg.theirCycleCounter);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
			streamMsg.action =  streamingService_ACKAndroidStream;
		  }
		  streamMsg.destination = androidTabLinkHandler.tabletAddress;
		  streamMsg.source      = DEFAULT_MAINBOARD_ADDRESS;
		  streamer_service_post(streamMsg); /* Send notification to the streamer task */
		  break;
		}
		case SERVICE_REQUEST_STOP_DATA_STREAM: /* Received STOP DATA stream message */
		{
		  if(serviceQueueMsg->status != measurement_StatusOriginMeasurementList) /* Request to stop data stream */
		  {
#if PRINTF_RTOS_UPLINK
			xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received request to stop data stream.\n", 0);
#endif
//			IMU_stop_measurements(&imu_1); // send message to BT1 to stop the IMU measurement (temporary solution)
			tablet_com_disable_streaming(); /* Disable streaming */
			osDelay(100);
			sendRequestAnswer(SERVICE_RESPONSE_STOP_DATA_STREAM, androidTabLinkHandler.tabletAddress); /* Answer to the stop data stream request */
		  }
		  else if(serviceQueueMsg->status == measurement_StatusOriginMeasurementList)
		  {
			if(serviceQueueMsg->action == measurement_StartMeasurement)
			{
			  if(app_storage_active())
			  {
#if PRINTF_RTOS_UPLINK
			    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received request to start data stream when a measurement from list is in progress.\n", 0);
#endif
			  }
			}
			else if(serviceQueueMsg->action == measurement_StopMeasurement)
			{
#if PRINTF_RTOS_UPLINK
			  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received request to stop data stream when a measurement from list is in progress.\n", 0);
#endif
			}
		  }
		  break;
		}
		case SERVICE_REQUEST_START_MEASUREMENT: /* Received START MEASUREMENT message */
		{
		if(serviceQueueMsg->status != measurement_StatusOriginMeasurementList)
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received Service request to start a measurement.\n", 0);
#endif
		  uint64_t epochNow_Ms = 0;
		  app_rtc_get_unix_epoch_ms(&epochNow_Ms); 										/* Get the unix time */
//	      epochNow_Ms = 0x000000DC6B069A80;												/* Saturday, January 1, 2000 2:00:00 AM GMT+01:00 */
//	      epochNow_Ms += HAL_GetTick();
		  app_rtc_set_cycle_counter(1); 												/* set cycle counter to 1 */
		  /* Send information to the measurement thread */
		  serviceQueueMsg->status 			= measurement_StatusOriginAndroidDevice;
		  serviceQueueMsg->action			= measurement_StartMeasurement;
		  serviceQueueMsg->measurementId	= 0;
		  serviceQueueMsg->startTime		= epochNow_Ms;
		  //measurements_service_post(*serviceQueueMsg); 									/* Send notification to the storage thread */
		  tablet_com_start_measurement_callback(epochNow_Ms); 							/* Answer to the start measurement request */
		}
		else
		{
		  //TODO
		  /* Send a message to notify that the action is not permitted. That means a measurement from list was already launched */
		}
		break;
	  }
	  case SERVICE_REQUEST_STOP_MEASUREMENT: /* Receive STOP MEASUREMENT message */
	  {
	    if(serviceQueueMsg->status != measurement_StatusOriginMeasurementList)
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received Service request to stop a measurement.\n", 0);
#endif
		  uint64_t epochNow_Ms = 0;
		  app_rtc_get_unix_epoch_ms(&epochNow_Ms); 								/* Get the epoch unix time from the RTC */
//	      epochNow_Ms = 0x000000DC6B069A80;												/* Saturday, January 1, 2000 2:00:00 AM GMT+01:00 */
//	      epochNow_Ms += HAL_GetTick();
		  serviceQueueMsg->action	= measurement_StopMeasurement;
		  serviceQueueMsg->stopTime = epochNow_Ms;
		  //measurements_service_post(*serviceQueueMsg); 							/* Send notification to the storage thread  */
		  memset(serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t));	/* Put the service Queue Msg in a known state*/
		  tablet_com_stop_measurement_callback(epochNow_Ms); 					/* Answer to the stop measurement request */
		  serviceQueueMsg->status   = measurement_StatusOriginIDLE;             /* put serviceQueueMsg back to IDLE */
		}
		else
		{
		  /* Send a message to notify that the action is not permitted */
		}
		break;
	  }
	  case SERVICE_RESPONSE_START_MEASUREMENT: /* Received START MEASUREMENT ACK from android device */
	  {
		if(serviceQueueMsg->status == measurement_StatusOriginMeasurementList) /* Received acknowledge from android device */
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received start measurement ACK. Enable streaming.\n", 0);
#endif
		  if(serviceQueueMsg->action == measurement_StartMeasurement)
		  {
			if (app_storage_active())
			{ /* If a measurement is ongoing, we should enable streaming */
// moved to app_init.c
//			  IMU_start_measurements_without_sync(&imu_1); // send message to BT1 to start the IMU measurement (temporary solution)
			  tablet_com_enabling_streaming(0, 0); /* Start measurement ACK received from android device: enable streaming, notify streamer */
			}
		  }
		  else
		  { /* Can't happen this means a bad implementation on the user side */
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received an ACK for START MEASUREMENT where it not supposed to.\n", 0);
#endif
		  }
		}
		break;
	  }
	  case SERVICE_RESPONSE_STOP_MEASUREMENT: /* Received STOP MEASUREMENT ACK from the android device */
	  {
		if(serviceQueueMsg->status == measurement_StatusOriginMeasurementList)
		{
		  if(serviceQueueMsg->action == measurement_StopMeasurement) /* Received acknowledge from android device */
		  {
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received stop measurement ACK disable streaming.\n", 0);
#endif
//			IMU_stop_measurements(&imu_1); // send message to BT1 to stop the IMU measurement (temporary solution)
			memset(serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t)); /* Put the serviceQueueMsg in a known state */
			tablet_com_disable_streaming(); /* Notify streamer thread to disable streaming */
			serviceQueueMsg->status = measurement_StatusOriginIDLE;         /* put serviceQueueMsg back to IDLE */
		  }
		  else if(serviceQueueMsg->action == measurement_StartMeasurement)
		  { /* This means that a measurement was in progress when a measurement list from MEAS List file was launched  */
#if PRINTF_RTOS_UPLINK
			xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received STOP Measurement ACK from the android device.\n", 0);
#endif
		  }
		  else
		  { /* Can't happen this means a bad implementation from the user side */
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received an ACK for STOP Measurement where its not supposed to.\n", 0);
#endif
		  }
		}
		break;
	  }
	  case SERVICE_REQUEST_SET_MEASUREMENT_ID: /* Received SET MEASUREMENT ID */
	  {
#if PRINTF_RTOS_UPLINK
	    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received SERVICE_REQUEST_SET_MEASUREMENT_ID.\n", 0);
#endif
		if(serviceQueueMsg->status != measurement_StatusOriginMeasurementList)
		{
#if PRINTF_RTOS_UPLINK
		  sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] Measurement ID : %u.\n",(unsigned int) buffer_to_unsigned_int(pMsg->DU));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  serviceQueueMsg->action = measurement_SetMeasurementID;
		  serviceQueueMsg->measurementId = buffer_to_unsigned_int(pMsg->DU);
		  measurements_service_post(*serviceQueueMsg); /* Send notification to storage thread  */
		}
		break;
	  }
	  case SERVICE_INFORMATION_USBAD_INSTRUMENT: /* Received Android device as instrument message */
	  {
	    if(pMsg->destinationAddress == DEFAULT_MAINBOARD_ADDRESS)
	    {
		  notification_service_post(&pMsg->DU[0], sizeof(usbad_data_t), 0); // Notify USB AD task
		}
		break;
	  }
	  case SERVICE_REQUEST_GET_JSON_SETUP: /* Get JSON message */
	  {
	    sendRequestAnswer(SERVICE_RESPONSE_GET_JSON_SETUP , androidTabLinkHandler.tabletAddress);
#if PRINTF_RTOS_UPLINK
		xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Received SERVICE_REQUEST_GET_JSON_SETUP.\n", 0);
#endif
		unsigned long length = 0;
		length = fs_get_JsonConfFile_size();
		if (length)
		  {
#if PRINTF_RTOS_UPLINK
		    sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] JSON File exists, length = %lu\n",(unsigned int) HAL_GetTick(), length);
			xQueueSend(pPrintQueue, string, 0);
#endif
			memset(pfcTxHandler, 0, sizeof(fc_tx_handler_t));
			pfcTxHandler->event 		= exampleEventCallback;
			pfcTxHandler->totalBytes 	= length;
			pfcTxHandler->payloadSize 	= 243; //todo The payload change in the new iteration
			pfcTxHandler->burstSize 	= 5;
			pfcTxHandler->ServiceID 	= SERVICE_RESPONSE_GET_JSON_SETUP;
			pfcTxHandler->destination 	= androidTabLinkHandler.tabletAddress | 0x80;
			pfcTxHandler->source 		= 0x81;
			pfcTxHandler->refreshData 	= refreshTxHandlerDataJson;
			pfcTxHandler->txData      	= UART3_WriteBytes;
			fc_tx_init(pfcTxHandler);
		  }
		  break;
		}
		default:
		{
#if PRINTF_RTOS_UPLINK
		  if (CplRxMsg.destinationService != 0x00)
		  {
			sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] Unknown Service Request with id %04x.\n",(unsigned int) HAL_GetTick(), CplRxMsg.SERVICE);
			xQueueSend(pPrintQueue, string, 0);
		  }
#endif
		  break;
		}
	  }
	  break;
	}
	case FLOW_CONTROL_FUNCTION_CODE_INIT: /* We will receive a bunch of packets using FC */
	{
#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] Initialize flow of packets to receive.\n", 0);
#endif
	  FC_PacketsExpected				= 0;
	  FC_ACK_Every 	   					= 0;
	  FC_PacketsExpected = ((pMsg->DU[0] << 24) | (pMsg->DU[1] << 16) | (pMsg->DU[2] << 8) | (pMsg->DU[3]));
	  FC_ACK_Every						= pMsg->DU[4];
//	  FC_TimeoutValue    				= xTaskGetTickCount();
	  pflowControlRx->Burst				=   pMsg->DU[4];
	  pflowControlRx->PacketsExpected	= ((pMsg->DU[0] << 24) | (pMsg->DU[1] << 16) | (pMsg->DU[2] << 8) | (pMsg->DU[3]));
	  pflowControlRx->ACK_Counter		= 0;
	  pflowControlRx->PacketsReceived	= 0;
	  pflowControlRx->Enabled			= 1;
	  switch (pMsg->SERVICE)
	  {
	    case SERVICE_REQUEST_RAW_SETUP:
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] Received a RAW SETUP Request\n", 0);
		  sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FC: Awaiting 0x%0X packets, have to ACK every 0x%0X RAW packets\n",pflowControlRx->PacketsExpected, pflowControlRx->Burst);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pflowControlRx->FC_State           = FC_RETRIEVING_RAW;
		  pflowControlRx->dataRxCallback     = fc_raw_data_rx_callback;
		  pflowControlRx->dataRxCpltCallback = fc_raw_data_rx_cplt_callback;
		  fifo_init_pt();						/* Initialize fifo which will contain data coming from android device */
		  rawConfigInit();						/* Prepare buffer to store configuration in RAM */
		  rawConfFile_HandleNewConf();			/* Prepare file that will store the raw configuration in NVM (SD card) */
		  flowInitACK(FC_PacketsExpected, androidTabLinkHandler.mainboardAddress, androidTabLinkHandler.tabletAddress, SERVICE_RESPONSE_RAW_SETUP);
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FlowInitACK sent.\n", 0);
#endif
		break;
		}
		case SERVICE_REQUEST_JSON_SETUP:
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] Received a JSON SETUP Request\n", 0);
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FC: Awaiting %d packets, have to ACK every %d JSON packets\n",(unsigned int) HAL_GetTick(), FC_PacketsExpected, FC_ACK_Every);
		  xQueueSend(pPrintQueue, string, 0);
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FC_State = FC_RETRIEVING_JSON.\n",(unsigned int) HAL_GetTick());
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pflowControlRx->FC_State           = FC_RETRIEVING_JSON;
		  pflowControlRx->dataRxCallback     = fc_json_data_rx_callback;
		  pflowControlRx->dataRxCpltCallback = fc_json_data_rx_cplt_callback;
		  fifo_init_pt();
		  jsonConfFile_HandleNewConf(); /* Prepare the file that will store the JSON configuration in NVM */
		  /* TODO: Delete the actual JSON File in SD if it exists */
  	  	  flowInitACK(FC_PacketsExpected, androidTabLinkHandler.mainboardAddress, androidTabLinkHandler.tabletAddress, SERVICE_RESPONSE_JSON_SETUP);
		  break;
		}
		case SERVICE_REQUEST_SEND_MEASUREMENT_LIST:
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] Received a SEND MEASUREMENT LIST Request.\n", 0);
		  sprintf(string, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FC: Awaiting %d packets, have to ACK every %d measurement list packets\n",pflowControlRx->PacketsExpected, pflowControlRx->Burst);
		  xQueueSend(pPrintQueue, string, 0);
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] FC_State = FFC_RETIEVING_MEAS_LIST.\n", 0);
#endif
		  pflowControlRx->FC_State = FC_RETIEVING_MEAS_LIST;
		  pflowControlRx->dataRxCallback     = fc_measurement_list_rx_callback;
		  pflowControlRx->dataRxCpltCallback = fc_measurement_list_data_rx_cplt_callback; /* Notify storage app that a new measurement list will come */
		  fifo_init_pt(); /* Init the fifo which will contain measurement list data coming from android device */
		  flowInitACK(FC_PacketsExpected, androidTabLinkHandler.mainboardAddress, androidTabLinkHandler.tabletAddress, SERVICE_RESPONSE_SEND_MEASUREMENT_LIST);
		  break;
		}
		default:
		{
#if PRINTF_RTOS_UPLINK
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FC = FLOW_CONTROL_FUNCTION_CODE_INIT] Unhandled message with id %04X.\n",(unsigned int) HAL_GetTick(), pMsg->SERVICE);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  break;
		}
	  }
	  break;
	}
	case FLOW_CONTROL_FUNCTION_CODE_DATA:
	{
#if PRINTF_RTOS_UPLINK
	  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Packets received = #%d, PacketsExpected = #%d.\n",(unsigned int) HAL_GetTick(), pflowControlRx->PacketsReceived, pflowControlRx->PacketsExpected);
	  xQueueSend(pPrintQueue, string, 0);
//	  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Received packet #%d\n", (unsigned int) HAL_GetTick(), pMsg->packetNumber);
//	  xQueueSend(pPrintQueue, string, 0);
#endif
	  if (pflowControlRx->Enabled)
	  {
	    pflowControlRx->PacketsReceived++;
		if (pMsg->packetNumber == (pflowControlRx->PacketsReceived)) /* This is the expected packet */
		{
#if PRINTF_RTOS_UPLINK
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Received packet #%d.\n",(unsigned int) HAL_GetTick(),pflowControlRx->PacketsReceived);
		  xQueueSend(pPrintQueue, string, 0);
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Status of the FC: Received %d/%d packets.\n",
						            (unsigned int) HAL_GetTick(), pflowControlRx->PacketsReceived, pflowControlRx->PacketsExpected);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  if (((pflowControlRx->PacketsReceived % pflowControlRx->Burst) == 0) || (pflowControlRx->PacketsExpected == pflowControlRx->PacketsReceived))
		  { /* Either burst or last message received */
#if PRINTF_RTOS_UPLINK
			sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Acknowledging every %d packets, packet Number: %d.\n",
								        (unsigned int) HAL_GetTick(), pflowControlRx->Burst, pMsg->packetNumber);
			xQueueSend(pPrintQueue, string, 0);
#endif
			flowACK(pflowControlRx->PacketsReceived, androidTabLinkHandler.tabletAddress, androidTabLinkHandler.mainboardAddress); /* Acknowledge in both cases */
		  }
		  /* This is the expected packet */
#if PRINTF_RTOS_UPLINK
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Status of the FC: Received %d/%d packets\n",(unsigned int) HAL_GetTick(), pflowControlRx->PacketsReceived, pflowControlRx->PacketsExpected);
		  HAL_Delay(20);
		  xQueueSend(pPrintQueue, string, 0);
		  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Received packet #%d\n", (unsigned int) HAL_GetTick(), pMsg->packetNumber);
		  HAL_Delay(20);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  if (pflowControlRx->dataRxCallback)
		  { /* The callback is not null */
			pflowControlRx->dataRxCallback(pMsg->DU+2, pMsg->lenght-7); /* TODO: handle the offset in msg due to packet # */
		  }
		  if ((pflowControlRx->PacketsExpected == pflowControlRx->PacketsReceived))
		  { /* Received all expected packets */
#if PRINTF_RTOS_UPLINK
		    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Received all expected packets.\n", 0);
#endif
			if (pflowControlRx->dataRxCallback)
			{ /* The callback is not null */
			  pflowControlRx->dataRxCpltCallback();
			  memset(pflowControlRx, 0, sizeof(fc_rx_handler_t));
			}
		  }
		}
		else
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] This is not the expected packet!\n", 0);
#endif
		  pflowControlRx->PacketsReceived--;
		  if (pMsg->packetNumber > (pflowControlRx->PacketsReceived+1)) /* NACK, because we expected packet N, but received a packet numbered after N */
		  {
#if PRINTF_RTOS_UPLINK
		    sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Received packet is #%d, expected packet was #%d.\n",
								        (unsigned int) HAL_GetTick(), pMsg->packetNumber,pflowControlRx->PacketsReceived+1);
			xQueueSend(pPrintQueue, string, 0);
#endif
//			ring_buffer_dequeue_arr(&cplRingbufRx, NULL, ring_buffer_num_items(&cplRingbufRx));
			unsigned int awaitedPacketNumber = pflowControlRx->PacketsReceived+1;
			flowNACK(awaitedPacketNumber, androidTabLinkHandler.tabletAddress, androidTabLinkHandler.mainboardAddress);
		  }
		}
	  }
	  else
	  {
#if PRINTF_RTOS_UPLINK
	    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_DATA] Error: NO_INIT\n", 0);
#endif
	  }
#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler [FLOW_CONTROL_FUNCTION_CODE_DATA] Receiving FC Data\n", 0);
#endif
	  break;
	}
	case FLOW_CONTROL_FUNCTION_CODE_ACK:
	{
#if PRINTF_RTOS_UPLINK
	  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_ACK] Received ack %d\n", (unsigned int) HAL_GetTick(),pMsg->packetNumber);
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_ACK] Current state of FC Transmit handler: %d\n", (unsigned int) HAL_GetTick(),pfcTxHandler->currentState);
	  xQueueSend(pPrintQueue, string, 0);
#endif
	  if (pfcTxHandler->currentState == FC_STATE_WAITING_INIT_PACKET_ANSWER) /* fcTxHandler was waiting for an answer to the init packet */
	  {
		unsigned short shiftedService = 0x0000;
		shiftedService = (pMsg->sourceService << 8) | pMsg->destinationService;
#if PRINTF_RTOS_UPLINK
		sprintf(string, "%u [APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_ACK] Shifted service: %04x\n",(unsigned int) HAL_GetTick(), shiftedService);
	    xQueueSend(pPrintQueue, string, 0);
#endif
		if (pfcTxHandler->ServiceID == shiftedService) /* Services matches, this is an answer to the correct service */
		{
#if PRINTF_RTOS_UPLINK
		  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] [FLOW_CONTROL_FUNCTION_CODE_ACK] Service matches.\n", 0);
#endif
		  pfcTxHandler->event(pfcTxHandler, FC_INIT_ACK, 0);
		}
	  }
	  else if (pfcTxHandler->currentState == FC_STATE_WAITING_BURST_ANSWER)
	  {
		/* Todo: check if the received ack (CplRxMsg.packetNumber) is equal to the awaited ack */
		pfcTxHandler->event(pfcTxHandler, FC_BURST_ACK, pMsg->packetNumber);
	  }
	  break;
	}
	default:
	{
#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [cp_ServiceHandler] Unknown FLOW_CONTROL_FUNCTION_CODE.\n", 0);
#endif
	}
  }
}

void fc_json_data_rx_callback(unsigned char * buffer, unsigned int length)
{
  for(unsigned int i = 0; i < length; i++)
  {
	fifo_pt_put(*(buffer + i));
  }
  if(fifo_pt_size() >= 1024) /* if the fifo is half full append data in the file */
  {
	unsigned int size = fifo_pt_size();
	unsigned char buffer[size];
	for(unsigned int i = 0; i < size; i++)
	{
	  fifo_pt_get(&buffer[i]);
	}
	jsonConfFile_Append((char*)buffer, size);
  }
}

void fc_json_data_rx_cplt_callback(void)
{
#if PRINTF_RTOS_UPLINK
  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_json_data_rx_cplt_callback] Started.\n", 0);
#endif
  unsigned int size = fifo_pt_size();
  unsigned char buffer[size];
  for(unsigned int i = 0; i < size; i++)
  {
	fifo_pt_get(&buffer[i]);
  }
  fifo_init_pt();
  jsonConfFile_Append((char*)buffer, size);
}

void fc_raw_data_rx_callback(unsigned char * buffer, unsigned int length)
{
#if PRINTF_RTOS_UPLINK
  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_callback] Started.\n", 0);
#endif
  for(unsigned int i = 0; i < length; i++)
  {
	fifo_pt_put(*(buffer + i));
  }
  if(fifo_pt_size() >= 1024) /* if the fifo is half full or more => append data in the file */
  {
#if PRINTF_RTOS_UPLINK
	xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_callback] fifo_pt_size() >= 1024.\n", 0);
#endif
	unsigned int size = fifo_pt_size();
	unsigned char buffer[size];
	for(unsigned int i = 0; i < size; i++)
	{
	  fifo_pt_get(&buffer[i]);
	}
	rawConfigAppend(buffer, size);
	rawConfFile_Append(buffer, size);
  }
  else
  {
#if PRINTF_RTOS_UPLINK
  sprintf(string, "[APP_TABLET_COM] [fc_raw_data_rx_callback] fifo_pt_size() < 1024, size = 0x%08X.\n",(unsigned int) fifo_pt_size());
  xQueueSend(pPrintQueue, string, 0);
#endif
  }
}

void fc_raw_data_rx_cplt_callback(void)
{
  unsigned int size = fifo_pt_size();
  unsigned char buffer[size];
#if PRINTF_RTOS_UPLINK
  HAL_Delay(20);
  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_cplt_callback] Started.\n", 0);
#endif
  for(unsigned int i = 0; i < size; i++)
  {
	fifo_pt_get(&buffer[i]);
  }
	fifo_init_pt();
	rawConfigAppend(buffer, size);
	rawConfFile_Append(buffer, size);
	if (rawConfFile_RetrieveConf()) /* Retrieve the configuration file retrieved and the decoding successful */
	{

#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_cplt_callback] Raw Configuration file decoded. Send Current Setup ID.\n", 0);
#endif
	  sendCurrentSetupID(getConfigId(), getConfigVersion(), androidTabLinkHandler.tabletAddress ); /* Send the msg */
	}
	else if(!rawConfFile_RetrieveConf()) /* Error, raw config received is not correct */
	{
#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_cplt_callback] Error, received RAW configuration file is not correct.\n", 0);
#endif
	}
	else if(rawConfFile_RetrieveConf() == 2) /* The configuration file is empty */
	{
#if PRINTF_RTOS_UPLINK
	  xQueueSend(pPrintQueue, "[APP_TABLET_COM] [fc_raw_data_rx_cplt_callback] Error, RAW configuration file is empty.\n", 0);
#endif
	}
}

void fc_measurement_list_rx_callback(unsigned char * buffer, unsigned int length)
{
  for(unsigned int i = 0; i < length; i++)
  {
    fifo_pt_put(*(buffer + i));
  }
  if(fifo_pt_size() >= 1024) /* if the fifo is half full append data in the file */
  {
	unsigned int size = fifo_pt_size();
	unsigned char buffer[size];
	for(unsigned int i = 0; i < size; i++)
	{
	  fifo_pt_get(&buffer[i]);
	}
	measurement_list_file_append(buffer, size);
  }
}

void fc_measurement_list_data_rx_cplt_callback(void)
{
  /* When finished interpreting data coming from the MEAS file list */
  //measurement_list_interpret_data_from_tablet(pMsg->DU);
  //measurement_list_read_file();
  uint32_t size = fifo_pt_size();
  unsigned char buffer[size];
  for(unsigned int i = 0; i < size; i++)
  {
	fifo_pt_get(&buffer[i]);
  }
  fifo_init_pt();
  if(measurement_list_file_append(buffer, size))
  {
	app_storage_notify(APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE); /* Send a notification to the measurement thread */
  }
}

void sendCurrentSetupID(unsigned int id, unsigned int version, unsigned int ad)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = ad | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	if ((id == 0) && (version == 0)) /* No config loaded: possibility A */
	{
		msg.destinationService = (SERVICE_REQUEST_SETUP_ID_FORCED >> 8) & 0x00ff;
		msg.sourceService      = SERVICE_REQUEST_SETUP_ID_FORCED 		& 0x00ff;
		msg.DU[0] = 0x00;
		msg.DU[1] = 0x00;
		msg.payloadLength = 2;
	}
	else /* A configuration is loaded */
	{
		msg.destinationService = (SERVICE_REQUEST_SETUP_ID >> 8) & 0x00ff;
		msg.sourceService      = SERVICE_REQUEST_SETUP_ID 		 & 0x00ff;
		msg.DU[0] = (unsigned char) ((unsigned short) id >> 8 & 0x00ff);
		msg.DU[1] = (unsigned char) ((unsigned short) id      & 0x00ff);
		msg.DU[2] = (unsigned char) ((unsigned short) version >> 8 & 0x00ff);
		msg.DU[3] = (unsigned char) ((unsigned short) version      & 0x00ff);
		msg.payloadLength = 4;
	}
	msg.lenght = 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [sendCurrentSetupID] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void cplAssignAddressID(unsigned char currentAddress, unsigned char addressAssigned, unsigned char sourceAddress)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = currentAddress | 0x80; /* Set the MSB */
	msg.sourceAddress      = sourceAddress;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService = (SERVICE_RESPONSE_CONNECTION_CHECK >> 8) & 0x00ff;
	msg.sourceService      = SERVICE_RESPONSE_CONNECTION_CHECK 		 & 0x00ff;
	msg.DU[0] = addressAssigned;
	msg.payloadLength = 1;
	msg.lenght = 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
	sprintf(string, "%u [APP_TABLET_COM] [cplAssignAddressID] Sending:",(unsigned int) HAL_GetTick());
    xQueueSend(pPrintQueue, string, 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void flowInitACK(unsigned int npackets, unsigned char sourceAd,unsigned char destAd, unsigned int dsap)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg,   0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = destAd   | 0x80; /* Set the MSB */
	msg.sourceAddress      = sourceAd | 0x80;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_ACK;
	msg.destinationService = (dsap >> 8) & 0x00ff;
	msg.sourceService      = dsap 		 & 0x00ff;
	msg.DU[0] = 0x00; /* Its the extra 0x00 byte */
	msg.DU[1] = (npackets >> 24) & 0xff;
	msg.DU[2] = (npackets >> 16) & 0xff;
	msg.DU[3] = (npackets >> 8)  & 0xff;
	msg.DU[4] = (npackets     )  & 0xff;
	msg.payloadLength = 5;
	msg.lenght = 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
	sprintf(string, "%u [APP_TABLET_COM] [flowInitACK] Sending:",(unsigned int) HAL_GetTick());
    xQueueSend(pPrintQueue, string, 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void flowACK(unsigned int npackets, unsigned char destination, unsigned char source )
{
	unsigned int numElems;
	unsigned char buffer[16];
	memset(buffer, 0, 16);
	numElems = fc_parse_frame_ack(destination, source, npackets, buffer);
#if PRINTF_RTOS_UPLINK
	sprintf(string, "%u [APP_TABLET_COM] [flowACK] Sending:",(unsigned int) HAL_GetTick());
    xQueueSend(pPrintQueue, string, 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes(buffer, numElems);
}

void flowNACK(unsigned int npackets, unsigned char destination, unsigned char source )
{
	unsigned int numElems;
	unsigned char buffer[16];
	memset(buffer, 0, 16);
	numElems = fc_parse_frame_nack(destination, source, npackets, buffer);
	if (numElems == 16)
	{
		UART3_WriteBytes(buffer, numElems);
	}
#if PRINTF_RTOS_UPLINK
	sprintf(string, "%u [APP_TABLET_COM] [flowNACK] Sending:",(unsigned int) HAL_GetTick());
    xQueueSend(pPrintQueue, string, 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes(buffer, numElems);
}

/* Send left part to the ID stored in 32bits unsigned integer as DSAP, right part as SSAP */
void sendRequestAnswer(unsigned int id, unsigned int ad)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg,   0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress 	= ad | 0x80; /* Set the MSB */
	msg.sourceAddress      	= 0x81;		 /* hard code for the moment */
	msg.FC                 	= FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService 	= (id >> 8) & 0x00ff;
	msg.sourceService 		= id 	    & 0x00ff;
	msg.payloadLength 		= 0;
	msg.lenght 				= 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [sendRequestAnswer] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

/* Send left part to the ID stored in 32bits unsigned integer as DSAP, right part as SSAP */
void sendStreamRequestAnswer(unsigned int id, unsigned int ad)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = ad | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService      = (id >> 8) & 0x00ff;
	msg.sourceService = id 	   & 0x00ff;
	msg.payloadLength = 1;
	msg.DU[0] = 0x80;
	msg.lenght = 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [sendStreamRequestAnswer] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

/* Send left part to the ID stored in 32bits unsigned integer as DSAP, right part as SSAP */
void streamAnswer(unsigned int id, unsigned int ad)
{
	/* Create the packet */
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = ad | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService      = (id >> 8) & 0x00ff;
	msg.sourceService = id 	   & 0x00ff;
	msg.payloadLength = 1;
	msg.DU[0] = 0x80; //todo: evaluate what code we send and rewrite the primitives
	msg.lenght = 0x05 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [streamAnswer] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void tablet_com_start_measurement_from_meas_list_callback(unsigned int measurementId)
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[32];
	memset(buffer, 0, 32);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = androidTabLinkHandler.tabletAddress  | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard coded for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService = (SERVICE_REQUEST_START_MEASUREMENT >> 8) & 0x00ff;
	msg.sourceService = SERVICE_REQUEST_START_MEASUREMENT 	   & 0x00ff;
	msg.payloadLength = 4;
	unsigned_int_to_byte_array(measurementId, msg.DU);
	msg.lenght = 5 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void tablet_com_stop_measurement_from_meas_list_callback(unsigned int measurementId)
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[32];
	memset(buffer, 0, 32);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = androidTabLinkHandler.tabletAddress  | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService = (SERVICE_REQUEST_STOP_MEASUREMENT >> 8) & 0x00ff;
	msg.sourceService = SERVICE_REQUEST_STOP_MEASUREMENT 	   & 0x00ff;
	msg.payloadLength = 4;
	unsigned_int_to_byte_array(measurementId, msg.DU);
	msg.lenght = 5 + msg.payloadLength;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

/* Implementation of the callback to refresh in case of Json transfer */
int refreshTxHandlerDataJson(struct fc_tx_handler_t * self)
{
#if PRINTF_RTOS_UPLINK
	sprintf(string, "%u [APP_TABLET_COM] [refreshTxHandlerDataJson] Got to copy %d bytes, from %d to %d\n", (unsigned int) HAL_GetTick(),self->bytesToProcess, self->startIndex, self->endIndex);
    xQueueSend(pPrintQueue, string, 0);
#endif
    self->bufferElems = self->bytesToProcess;
    unsigned int size   = self->bytesToProcess;
    unsigned int offset = self->startIndex;
    unsigned int returnSize = 0;
    fs_read_JsonConfFile(offset, size, self->buffer, &returnSize);
    return 1;
}

void tablet_com_send_shutdown_message()
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[16];
	memset(buffer, 0, 16);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = androidTabLinkHandler.tabletAddress | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService = (SERVICE_REQUEST_SHUTDOWN >> 8) & 0x00ff;
	msg.sourceService = SERVICE_REQUEST_SHUTDOWN 	   & 0x00ff;
	msg.payloadLength = 0;
	msg.lenght = 0x05;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void tablet_com_start_measurement_callback(uint64_t startTime)
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[32];
	memset(buffer, 0, 32);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress = androidTabLinkHandler.tabletAddress  | 0x80; /* Set the MSB */
	msg.sourceAddress      = 0x81;		 /* hard code for the moment */
	msg.FC                 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService = (SERVICE_RESPONSE_START_MEASUREMENT >> 8) & 0x00ff;
	msg.sourceService = SERVICE_RESPONSE_START_MEASUREMENT 	   & 0x00ff;
	msg.payloadLength = 8;
	uint64t_to_bytes_array(startTime, msg.DU);
	msg.lenght = 0x05 + 8;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [tablet_com_start_measurement_callback] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void app_tablet_com_prepare_for_shutdown(void)
{
	tablet_com_send_shutdown_message();
}

void tablet_com_stop_measurement(uint64_t stopTime)
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[32];
	memset(buffer, 0, 32);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress 	= androidTabLinkHandler.tabletAddress  | 0x80; /* Set the MSB */
	msg.sourceAddress      	= 0x81;		 /* hard code for the moment */
	msg.FC                 	= FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService 	= (SERVICE_REQUEST_STOP_MEASUREMENT >> 8) & 0x00ff;
	msg.sourceService 		= SERVICE_REQUEST_STOP_MEASUREMENT 	   & 0x00ff;
	msg.payloadLength 		= 8;
	uint64t_to_bytes_array(stopTime, msg.DU);
	msg.lenght = 0x05 + 8;/* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

void tablet_com_stop_measurement_callback(uint64_t stopTime)
{
	cpl_msg_t msg;
	unsigned int numElems;
	char buffer[32];
	memset(buffer, 0, 32);
	memset(&msg, 0, sizeof(cpl_msg_t));
	/* Build */
	msg.destinationAddress 	= androidTabLinkHandler.tabletAddress  | 0x80; /* Set the MSB */
	msg.sourceAddress      	= 0x81;		 /* hard code for the moment */
	msg.FC                 	= FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	msg.destinationService	= (SERVICE_RESPONSE_STOP_MEASUREMENT >> 8) & 0x00ff;
	msg.sourceService 		= SERVICE_RESPONSE_STOP_MEASUREMENT 	   & 0x00ff;
	msg.payloadLength 		= 8;
	uint64t_to_bytes_array(stopTime, msg.DU);
	msg.lenght = 0x05 + 8; /* its 0x05 + the DU length */
	cpl_buildFrame(&msg, &numElems, buffer);
#if PRINTF_RTOS_UPLINK
    xQueueSend(pPrintQueue, "[APP_TABLET_COM] [tablet_com_stop_measurement_callback] Sending:", 0);
    sprintf(string, " ");
    char DUString[3];
    for (unsigned int i = 0; i<numElems; i++)
    {
      if (strlen(string) < 147)
      {
	   	  sprintf(DUString, "%02X",buffer[i]);
	   	  strcat(string, DUString);
      }
    }
	strcat(string,"\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	UART3_WriteBytes((uint8_t*)buffer, numElems);
}

int tablet_com_is_online(void)
{
	return androidTabLinkHandler.isOnline;
}

int tablet_com_set_state(unsigned int state)
{
	if(state >= 1)
	{
		androidTabLinkHandler.isOnline = 1;
	}
	else
	{
		androidTabLinkHandler.isOnline = 0;
	}
}

/**
 * @fn void tablet_com_enabling_streaming(unsigned char nPacket, unsigned short nBytes)
 * @brief Send a notification to the streamer task in order to enable the streaming
 * @param
 * @param
 * @return
 */
void tablet_com_enabling_streaming(unsigned char nPacket, unsigned short nBytes)
{
//  streamerServiceQueueMsg_t streamMsgEnabled;
  streamMsgEnabled.action 			= streamerService_EnableAndroidStream;
  streamMsgEnabled.source      		= DEFAULT_MAINBOARD_ADDRESS;
  streamMsgEnabled.destination 		= androidTabLinkHandler.tabletAddress;
  streamMsgEnabled.nPackets 		= nPacket;
  streamMsgEnabled.nBytes 			= nBytes;
  streamer_service_post(streamMsgEnabled);
}

void tablet_com_disable_streaming(void)
{
//  streamerServiceQueueMsg_t streamMsg;
  streamMsgEnabled.action 			= streamerService_DisableAndroidStream;
  streamMsgEnabled.source      		= DEFAULT_MAINBOARD_ADDRESS;
  streamMsgEnabled.destination 		= androidTabLinkHandler.tabletAddress;
  streamer_service_post(streamMsgEnabled);
}

void watchdog_timer_init(void) {
  xWatchdogTimerHandle = xTimerCreate(
						    "WatchdogTimer",		/* Just a text name, not used by the RTOS kernel. */
							50,						/* The timer period in ticks, must be greater than 0. */
							pdTRUE,					/* The timers will auto-reload themselves when they expire. */
							(void *) 0,				/* The ID is used to store a count of number of times timer has expired, which is initialized to 0. */
							watchdogTimerCallback); /* Each timer calls the same callback when it expires. */
  if (xWatchdogTimerHandle == NULL)
  { /* The timer was not created. */
  }
  else
  { /* Start the timer. No block time is specified and even if one was it would be ignored because the RTOS scheduler has not yet been started. */
	if (xTimerStart(xWatchdogTimerHandle, 0) != pdPASS)
	{ /* The timer could not be set into the Active state. */
	}
  }
}

void watchdogUsbRxCallback(void)
{
  tLastWatchdogMsg = xTaskGetTickCount();
}

void watchdogSetTime(uint32_t watchdogValueInms)
{
  tLastWatchdogMsg = watchdogValueInms;
}

uint32_t watchdogGetTime(void)
{
  return tLastWatchdogMsg;
}

void watchdogTimerCallback(TimerHandle_t xTimer)
{
  uint32_t timewithoutWatchdog = xTaskGetTickCount() - watchdogGetTime();
  if(tablet_com_is_online() && (!app_streamer_usb_stream_enabled()) && (!watchdogGetTime()))
  { /* First message received from android device */
	watchdogUsbRxCallback(); /* We first must initialize the watchdog value */
  }
  else if(tablet_com_is_online() && timewithoutWatchdog >= USB_TABLET_COM_WATCHDOG_TIMEOUT)
	{ /* If android device is connected and watchdog timeout is exceeded */
// Do not do anything yet with this... 20210505
#if PRINTF_RTOS_UPLINK
  sprintf(string, "%u [APP_TABLET_COM] [watchdogTimerCallback] TIMEOUT: %d (time is %u, last watchdog rxd:%u)\n",
			         (unsigned int) HAL_GetTick(),
					 (unsigned int) timewithoutWatchdog,
					 (unsigned int) xTaskGetTickCount(),
					 (unsigned int) watchdogGetTime());
  xQueueSend(pPrintQueue, string, 0);
#endif
//		tablet_com_set_state(0); // Pass the status of the android device to false
//		watchdogSetTime(0); // Reset the watchdog
//		firstAvailableAddress--; // Decrement the first available address
//		if(app_streamer_usb_stream_enabled())
//		{
//			/* Disable the streaming if enable */
//			tablet_com_disable_streaming();
//		}
//		if(app_storage_active())
//		{
//			/* Disable the storage if enable */
//		}
  }
  configASSERT(xTimer); /* Optionally do something if the pxTimer parameter is NULL. */
}

void tablet_com_send_watchdog_msg(char ad)
{
  cpl_msg_t msg;
  unsigned int numElems;
  char buffer[16];
  memset(buffer, 0, 16);
  memset(&msg, 0, sizeof(cpl_msg_t));
  /* Build */
  msg.destinationAddress	= ad | 0x80;									/* Set the MSB */
  msg.sourceAddress			= 0x81;											/* hard code for the moment */
  msg.FC					= FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
  msg.destinationService	= (SERVICE_SOFTWARE_WATCHDOG >> 8) & 0x00ff;
  msg.sourceService 		= SERVICE_SOFTWARE_WATCHDOG & 0x00ff;
  msg.payloadLength			= 0;
  msg.lenght				= 0x05;											/* its 0x05 + the DU length */
  cpl_buildFrame(&msg, &numElems, buffer);
  UART3_WriteBytes((uint8_t*)buffer, numElems);
}
