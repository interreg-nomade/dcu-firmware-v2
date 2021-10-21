/**
 * @file app_streamer.c
 * @brief Streaming to tablet application
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 */
#include "app_streamer.h"
#include "app_tablet_com.h"
#include "app_rtc.h"

#include <data/structures.h>  /* Contains the data structures */
#include "pUART.h"
#include <string.h>
#include "common.h"
#include "tablet_com_protocol/parser.h"	/* Export to the Android tablet */
#include "queues/streamer_service/streamer_service_queue.h"
#include "tablet_com_protocol/streaming.h"

#include "FreeRTOS.h"
#include "cmsis_os.h"

#define PRINTF_APP_STREAMER 0
#define PRINT_APP_STREAMER_DBG_MSG	0
#if PRINT_APP_STREAMER_DBG_MSG
#define PRINT_APP_STREAMER_ACK_VALUE				0
#define PRINT_APP_STREAMER_RECEIVED_CC				0
#define PRINT_APP_STREAMER_SENDING_CC				0
#endif

#define STREAMER_ACK_TIMEOUT 60 /* counted in term of ticks of 20ms. e.g. value of 3 here = 60ms - counted in ms? */

unsigned int streamn = 0;
unsigned char streambuffer[1024];

static streamingPort_handler_t USB_Android_Port_Handler;
static osThreadId streamerThreadHandle;

extern char string[];
extern QueueHandle_t pPrintQueue;

void initStreamerThread(void)
{
  osThreadDef(tabletDataStreamThread, tabletDataStreamThread, osPriorityNormal, 0, 2048); /* Tablet Link thread definition  */
  streamerThreadHandle = osThreadCreate (osThread(tabletDataStreamThread), NULL); /* Start tablet link thread  */
}

void tabletDataStreamThread(const void *params)
{
  uint32_t ulNotificationValue = 0; 						/* Used for the notification coming from the app sync thread */
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(400);		/* Block 400 ms */
  unsigned int ltLastAck = 0; 								/* Used to handle the ack timeout */
  uint64_t epochNow_Ms = 0; 								/* Unix epoch in ms */
  streamerServiceQueueMsg_t service; 						/* Initialize variable of transmission  */
  bool stream_init_done = false;							/* to make sure we initialize pHandler only once*/
  stream_init(&USB_Android_Port_Handler);
  for(;;)
  {
	if (streamer_service_receive(&service))
	{
//#if PRINTF_APP_STREAMER
//	  sprintf(string, "[app_streamer] [tabletDataStreamThread] Received service %d\n",service);
//	  QueueSend(pPrintQueue, string, 0);
//#endif
	  switch (service.action)
	  {
		case streamerService_DisableAll:
		{
		  break;
		}
		case streamerService_DisableAndroidStream:
		{
		  USB_Android_Port_Handler.state = streamingPort_DISABLED;
		  break;
		}
		case streamerService_EnableAndroidStream:
		{
//#if PRINTF_APP_STREAMER
//	      xQueueSend(pPrintQueue, "[app_streamer] [tabletDataStreamThread] Enable Android Stream.\n", 0);
//#endif
//		 if (!stream_init_done)
//		 {
			stream_init(&USB_Android_Port_Handler);
			stream_init_done = true;
//		 }
		  app_rtc_set_cycle_counter(1); 												/* set cycle counter to 1 */

		  USB_Android_Port_Handler.bytesPerPackets 		  = STREAMING_PACKET_DEFAULT_PAYLOAD_SIZE;
		  USB_Android_Port_Handler.streamingPacketsNumber = service.nPackets;
		  USB_Android_Port_Handler.streamingPacketsbytes  = service.nBytes;
		  USB_Android_Port_Handler.destination 			  = service.destination;
		  USB_Android_Port_Handler.source				  = service.source;
		  USB_Android_Port_Handler.ourCycleCounter 		  = 0; //todo: move in the init function
		  USB_Android_Port_Handler.theirCycleCounter	  = 0; //todo: move in the init function
		  USB_Android_Port_Handler.dsap                   = SERVICE_REQUEST_PACKET_DATA_STREAM >> 8;
		  USB_Android_Port_Handler.ssap                   = SERVICE_REQUEST_PACKET_DATA_STREAM;
		  USB_Android_Port_Handler.txData   			  = UART3_WriteBytes;
		  //USB_Android_Port_Handler.txData   			  = stream_tester_output; // this function outputs on the printf for dbg purposes
		  USB_Android_Port_Handler.txDbgData 			  = NULL;
		  USB_Android_Port_Handler.state 				  = streamingPort_ENABLED;
#if PRINTF_APP_STREAMER
		  sprintf(string, "[app_streamer] [tabletDataStreamThread] Stream setup: number of packets per stream burst: %d, number of bytes in total: %d\n",
				  service.nPackets, service.nBytes);
		  xQueueSend(pPrintQueue, string, 0);
#endif

		  //stream_setup(&USB_Android_Port_Handler); // only thing what this is doing is to put the cycle counters to 0...

		  ltLastAck = xTaskGetTickCount();
#if PRINT_APP_STREAMER_ACK_VALUE
			printf("[APP STREAMER]Start Streaming ASKED initialization is done the timeout for the streaming ACK is : %d\n",
					ltLastAck);
#endif
		  break;
		}
		case streamerService_DisableUsbStream:
		{
		  break;
		}
		case streamerService_EnableUsbStream:
		{
		  break;
		}
		case streamingService_ACKAndroidStream: /* We received an ACK from the android stream, this part is handled in the app_tablet_com thread */
		{
			ltLastAck = xTaskGetTickCount(); /* Update the ACK timeout */
#if PRINT_APP_STREAMER_ACK_VALUE
		      sprintf(string,"[APP STREAMER]Receive streaming ACK the timeout is now at : %d\n", ltLastAck);
		      xQueueSend(pPrintQueue, string, 0);
#endif
#if PRINTF_APP_STREAMER_RECEIVED_CC
	      sprintf(string, "[app_streamer] [tabletDataStreamThread] Cycle counter received: %d, Previous Cycle counter received : %d\n",
						service.theirCycleCounter,
						USB_Android_Port_Handler.theirCycleCounter);
	      xQueueSend(pPrintQueue, string, 0);
#endif
  		  if (service.theirCycleCounter <= USB_Android_Port_Handler.theirCycleCounter)
		  {
		    /* Received an ack with a cycle counter lower than expected */
			//TODO: what to do?
#if PRINTF_APP_STREAMER
	        sprintf(string, "[app_streamer] [tabletDataStreamThread] ACK received, service theirCycleCounter = %d, while USB_Android_Port_Handler.theirCycleCounter = %d.\n",
	        		service.theirCycleCounter, USB_Android_Port_Handler.theirCycleCounter);
	        xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  else
		  {
			USB_Android_Port_Handler.theirCycleCounter = service.theirCycleCounter;
//#if PRINTF_APP_STREAMER
//	      sprintf(string, "[app_streamer] [tabletDataStreamThread] ACK received, cycle counter: 0x%08X\n",USB_Android_Port_Handler.theirCycleCounter);
//	      xQueueSend(pPrintQueue, string, 0);
//#endif
		  }
		  break;
		}
		case streamingService_NACKAndroidStream:
		{
			/* TODO : Handle the NACK functionality in the android streaming */
			break;
		}
		default:
		{ /* NOT HANDLED */
		  break;
		}
	  }
	}
	/* Block until we get the notification from the app sync  */
	if (xTaskNotifyWait( 0x00,               		/* Don't clear any bits on entry. */
						 0xffffffff,          		/* Clear all bits on exit. (long max) */
						 &ulNotificationValue, 		/* Receives the notification value. */
						 xMaxBlockTime ) == pdTRUE)	/* Block 400 ms */
	{
	  if ((ulNotificationValue & APP_STREAMER_NOTIF_CYCLE_COUNTER) == APP_STREAMER_NOTIF_CYCLE_COUNTER)
	  { /* Received a notification from app_sync */
	    if (((xTaskGetTickCount() - ltLastAck) > STREAMER_ACK_TIMEOUT) && (USB_Android_Port_Handler.state == streamingPort_ENABLED ))
		{ /* First if the streaming is enable -> check for the ACK timeout */
//		  USB_Android_Port_Handler.state = streamingPort_DISABLED;
		}
		if (USB_Android_Port_Handler.state == streamingPort_ENABLED)
		{ /* If the streaming is still enabled send the data to the Android device */
		  if (USB_Android_Port_Handler.comState == streamingPort_COM_AVAILABLE)
		  {
			USB_Android_Port_Handler.ourCycleCounter = snapshotconf.cycleCounter;
			if (config_createStreamPacket(&snapshotconf, streambuffer, &streamn))
			{ /* we can manipulate the buffer and process it */
			  if(streamn)
			  {
//#if PRINTF_APP_STREAMER
//	        sprintf(string, "[app_streamer] [tabletDataStreamThread] Sending cycle counter 0x%08X.\n",(unsigned int)snapshotconf.cycleCounter);
//		    xQueueSend(pPrintQueue, string, 0);
//#endif
			    stream_load(&USB_Android_Port_Handler, streambuffer, streamn); 		/* load the stream buffer */
				stream_send(&USB_Android_Port_Handler); 							/* send the stream buffer */
			  }
			}
		  }
		}
	  }
	}
  }
}

int app_streamer_usb_stream_enabled(void)
{
  return (USB_Android_Port_Handler.state);
}

void app_streamer_notify_from_isr(uint32_t notValue)
{
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  if (streamerThreadHandle != NULL)
  {
	xTaskNotifyFromISR(streamerThreadHandle, notValue, eSetBits, &xHigherPriorityTaskWoken);
  }
  portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

void app_streamer_notify(uint32_t notValue)
{
  if (streamerThreadHandle != NULL)
  {
  xTaskNotify(streamerThreadHandle, notValue, eSetBits);
  }
}
