/*
 * app_BT1_com.c
 *
 *  Created on: 19 Mar 2021
 *      Author: sarah
 */


#include "app_BT1_com.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usart.h"
#include "../lib/ring_buffer/ringbuffer_char.h"
#include <string.h>
#include "../Inc/usb_com.h"
#include "data/structures.h"
#include "app_terminal_com.h"
#include "app_imu.h"
#include "queues/streamer_service/streamer_service_queue.h"
#include "config/parameters_id.h"

#define PRINTF_BT_COMMANAGER 1
#define PRINTF_BT_COM 0

/* RTOS Variables */
static osThreadId BT1RxTaskHandle;

/* Com link protocol variables */
static BT_msg_t BT1RxMsg;
static ring_buffer_t BT1RingbufRx;
static int BTChannel = 7;

extern streamerServiceQueueMsg_t streamMsgEnabled;


extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];

extern char string[];
extern QueueHandle_t pPrintQueue;

extern QueueHandle_t eventQueue;
extern SemaphoreHandle_t wakeSensorTask;

imu_module *imuBT1 = NULL;
uint8_t previous_connected_modules [6];

char command;

void BTComManagerThread(const void *params);

/* Declare private functions */
static int bt_FindSdByte(void);
static int bt_HeaderPartPresent(void);
static int bt_BuildHeader(void);
static int bt_BodyPartPresent(void);
static int bt_BuildBody(void);
static int bt_csCorrect(void);
static int bt_RecMsgNotOK(void);
//static void rsv_data_msg_handler(uint8_t * rsvbuf, uint8_t len, imu_module *imu);
static void rsv_data_msg_handler(void);
static void rsv_data_handler(uint8_t * buf, uint8_t sensor_number, uint8_t data_format);

imu_100Hz_data_t sensorEvent;

void bt_init_rx_task(void)
{
	osDelay(100);
    /* Init the decoder */
    bt_init();

#if PRINTF_BT_COMMANAGER

    sprintf(string, "BT1_com_init_rx_task done. \n");
    HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
#endif

    memset(&BT1RxMsg, 0, sizeof(BT_msg_t));

    osThreadDef(BTComManager, BTComManagerThread, osPriorityNormal, 0, 1024);
    BT1RxTaskHandle = osThreadCreate(osThread(BTComManager), NULL);

}

void BTComManagerThread(const void *params)
{
  /* Initialize the local container */
  osDelay(100);
//  bt_init();
  for(;;)
  {
	BT_DECODER_RESULT BTRes = bt_prot_decoder(&BT1RxMsg);
    if (BTRes == BT_CORRECT_FRAME)
	{
      if (BT1RxMsg.command != CMD_DATA_IND)
      {
#if PRINTF_BT_COMMANAGER
	    sprintf(string, "%u [app_BT1_com] [BTComManagerThread] BT module ",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
      }
	  switch (BT1RxMsg.command)
	  {
		case CMD_DATA_IND: //  Data has been received
		{
          // find out from which channel data is received...
          int match = 0;
          int MACZero = 0;
          int ChannelFound = 6;
          for (int i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++) // check for all available BT channels
          {
            for (int j = 0; j < 6; j++)
            {
              if (imu_array[i]->mac_address[j] == BT1RxMsg.DU[j])
              {
      	        match++;
      	        MACZero += imu_array[i]->mac_address[j];
      	      }
            }
            if (match == 6)
            {
              if (MACZero != 0)
              {
                ChannelFound = i;
                MACZero = 0;
              }
            }
          }
          if (ChannelFound != 6)
          {
            BTChannel = ChannelFound;
//#if PRINTF_BT_COMMANAGER
//    	    sprintf(string, "%d data has been received with RSSI of 0x%02X (%d dBm). Length of data is %d\n",imu_array[ChannelFound]->number,BT1RxMsg.DU[6],(int)BT1RxMsg.DU[6],BT1RxMsg.length);
//  	        xQueueSend(pPrintQueue, string, 0);
//#endif
            if (BT1RxMsg.length > 12)
            { /* data received from measurement */
      	      if (streamMsgEnabled.action == streamerService_EnableAndroidStream)
     	      {
//#if PRINTF_BT_COMMANAGER
//                //sprintf(string, "%u [app_BT1_com] Payload IMU: ", (unsigned int) HAL_GetTick());
//                sprintf(string, "%u Payload: ", (unsigned int) HAL_GetTick()); // shorter sentence to print as much as possible...
//      	        char DUString[3];
//      	        for (int i = 7; i < BT1RxMsg.length; i++)
//      	        {
//      	    	  if (strlen(string) < 149)
//      	    	  {
//          	    	  sprintf(DUString, "%02X",BT1RxMsg.DU[i]);
//          	    	  strcat(string, DUString);
//      	    	  }
//      	        }
//       	        strcat(string,"\n");
//      	        xQueueSend(pPrintQueue, string, 0);
//#endif
      	        int16_t data [20];
      	        uint32_t timestamp;
      	        uint16_t pakket_send_nr;
     	        int data_values = 0;
     	        int data_reads = 0;
        	    switch(imu_array[BTChannel]->outputDataType)
        	    {
        	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
        	      {
         	        data_values = 4;
         	        data_reads = NUMBER_OF_DATA_READS_IN_BT_PACKET;
         	        break;
        	      }
        	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
				  { // battery voltage level not yet implemented, same as quaternions only for the moment
	         	    data_values = 4;
         	        data_reads = NUMBER_OF_DATA_READS_IN_BT_PACKET;
	         	    break;
				  }
        	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
				  {
		         	data_values = 10;
         	        data_reads = NUMBER_OF_DATA_READS_IN_BT_PACKET;
		         	break;
				  }
        	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
        	      {
  		         	data_values = 20;
         	        data_reads = NUMBER_OF_DATA_READS_IN_BT_PACKET / 2;
  		         	break;
        	      }
        	      default:
        	      {
        	    	// wrong value, report error
        	    		// todo
        	      }
        	    }
      	        int start = 7;
      	        int value = 0;
      	        uint16_t start_pos = start + 7;
      	        pakket_send_nr = BT1RxMsg.DU[start + 1] | (BT1RxMsg.DU[start + 2] << 8);
      	        timestamp = BT1RxMsg.DU[start + 3] | (BT1RxMsg.DU[start + 4] << 8) | (BT1RxMsg.DU[start + 5] << 16) | (BT1RxMsg.DU[start + 6] << 24);

//      	      sprintf(string, "[app_BT1_com] sample frequency: %u \n", (unsigned int) imu_array[0]->sampleFrequency);
//      	      xQueueSend(pPrintQueue, string, 0);
        	    for(int i = 0; i < data_reads; i++)
        	    {
        	      for(uint8_t g = 0; g < data_values; g++)
        	      {
        	        data[g] = ((BT1RxMsg.DU[start_pos + g*2 + data_values*2 * i] << 8) | BT1RxMsg.DU[start_pos + g*2 + 1 + data_values*2 * i]);
        	      }
  //          	  switch(imu_array[BTChannel]->sampleFrequency)
  //          	  {
  //          		case 10:  value = 5;  break;
  //          		case 20:  value = 5;  break; //-> not possible at the moment! (20Hz is 50ms, not a multiple of 20ms!)
  //          		case 25:  value = 2;  break;
  //          		case 50:  value = 1;  break;
  //          		case 100: value = 1;  break; //-> not possible at the moment! (100Hz is 10ms, speed is max 20ms!)
  //          		default:
  //          		{ /* if sample frequency is not a defined value, the sample frequency is set to 50Hz */
            		value = 1;
  //          	    }
  //          	  }
            	  for (uint8_t f = 0; f < value; f++)
            	  {
            	    switch(imu_array[BTChannel]->outputDataType)
            	    {
            	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
            	      {
                        sensorEvent.rotVectors1.real = data[0];
                        sensorEvent.rotVectors1.i 	 = data[1];
                        sensorEvent.rotVectors1.j 	 = data[2];
                        sensorEvent.rotVectors1.k 	 = data[3];
                        sensorHandler(&sensorEvent);
           	    		break;
            	      }
            	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
					  { // battery voltage level not yet implemented, same as quaternions only for the moment
	                    sensorEvent.rotVectors1.real = data[0];
	                    sensorEvent.rotVectors1.i 	 = data[1];
	                    sensorEvent.rotVectors1.j 	 = data[2];
	                    sensorEvent.rotVectors1.k 	 = data[3];
	                    sensorHandler(&sensorEvent);
	           	    	break;
					  }
            	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
					  {
		                sensorEvent.rotVectors1.real = data[0];
		                sensorEvent.rotVectors1.i 	 = data[1];
		                sensorEvent.rotVectors1.j 	 = data[2];
		                sensorEvent.rotVectors1.k 	 = data[3];
		                sensorEvent.gyroscope1.x	 = data[4];
		                sensorEvent.gyroscope1.y	 = data[5];
		                sensorEvent.gyroscope1.z	 = data[6];
		                sensorEvent.accelerometer1.x = data[7];
		                sensorEvent.accelerometer1.y = data[8];
		                sensorEvent.accelerometer1.z = data[9];
		                sensorHandler(&sensorEvent);
		           	    break;
					  }
            	      case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
            	      {
  		                sensorEvent.rotVectors1.real = data[0];
  		                sensorEvent.rotVectors1.i 	 = data[1];
  		                sensorEvent.rotVectors1.j 	 = data[2];
  		                sensorEvent.rotVectors1.k 	 = data[3];
  		                sensorEvent.gyroscope1.x	 = data[4];
  		                sensorEvent.gyroscope1.y	 = data[5];
  		                sensorEvent.gyroscope1.z	 = data[6];
  		                sensorEvent.accelerometer1.x = data[7];
  		                sensorEvent.accelerometer1.y = data[8];
  		                sensorEvent.accelerometer1.z = data[9];
  		                sensorEvent.rotVectors2.real = data[10];
  		                sensorEvent.rotVectors2.i 	 = data[11];
  		                sensorEvent.rotVectors2.j 	 = data[12];
  		                sensorEvent.rotVectors2.k 	 = data[13];
  		                sensorEvent.gyroscope2.x	 = data[14];
  		                sensorEvent.gyroscope2.y	 = data[15];
  		                sensorEvent.gyroscope2.z	 = data[16];
  		                sensorEvent.accelerometer2.x = data[17];
  		                sensorEvent.accelerometer2.y = data[18];
  		                sensorEvent.accelerometer2.z = data[19];
  		                sensorHandler(&sensorEvent);
  		           	    break;
            	      }
            	      default:
            	      {
            	    	// wrong value, report error
            	    		// todo
            	      }
            	    }
//  #if PRINTF_BT_COMMANAGER
//                    sprintf(string, "%u [app_BT1_com] sensor event data to queue: ", (unsigned int) HAL_GetTick());
//        	          char DUString[6];
//        	          for (int s = 0; s < 4; s++)
//        	          {
//            	    	sprintf(DUString, "%04X ",data[s]);
//            	    	strcat(string, DUString);
//         	          }
//         	          strcat(string,"\n");
//        	          xQueueSend(pPrintQueue, string, 0);
//  #endif
            	  }
        	    }
      	      }
            }
            else
            {
              rsv_data_msg_handler();
            }
          }
          else
          {
#if PRINTF_BT_COMMANAGER
	        xQueueSend(pPrintQueue, "not found. Restart modules.\n", 0);
#endif
          }
		}
		break;
		case CMD_CONNECT_IND: //  Connection established
		{
        if (!BT1RxMsg.DU[0]) // Received status is OK
        {
#if PRINTF_BT_COMMANAGER
        	sprintf(string, "connection established. Connected device MAC address = 0x");
  	      char DUString[3];
          for (int i = 6; i > 0; i--)
          {
            sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
	    	strcat(string, DUString);
          }
          strcat(string, ".\n");
  	      xQueueSend(pPrintQueue, string, 0);
#endif
        }
        else // Received status is not OK
        {
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "connection failed. This could be due to a timeout (as defined by RF_ScanTiming).\n", 0);
#endif
        }
		}
		break;
		case CMD_RESET_CNF: //  BT module Reset request received
		{
#if PRINTF_BT_COMMANAGER
   	      xQueueSend(pPrintQueue, "reset request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
    	    xQueueSend(pPrintQueue, ", will perform test now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_GETSTATE_CNF: //  Return the BT module state
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "state request received", 0);
#endif
		  switch(BT1RxMsg.DU[0]) // Received BT module role
		  {
		    case 0x00: // BT module has no role
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module has no role.\n", 0);
#endif
	          break;
            }
		    case 0x01: // BT module has peripheral role
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module has peripheral role.\n", 0);
#endif
	          break;
            }
		    case 0x02: // BT module has central role
		    {
#if PRINTF_BT_COMMANAGER
	          sprintf(string, ", module has central role.\n");
	  	      xQueueSend(pPrintQueue, string, 0);
#endif
	          break;
            }
		    case 0x10: // BT module is in Direct Test Mode (DTM)
		    {
#if PRINTF_BT_COMMANAGER
	          sprintf(string, ", module is in Direct Test Mode (DTM).\n");
	  	      xQueueSend(pPrintQueue, string, 0);
#endif
		    }
		    default: // Reserved
		    {
#if PRINTF_BT_COMMANAGER
	          sprintf(string, ", Reserved module status.\n");
	  	      xQueueSend(pPrintQueue, string, 0);
#endif
		    }
		  }
#if PRINTF_BT_COMMANAGER
		  sprintf(string, "%u [app_BT1_com] [BTComManagerThread] BT module state request received",(unsigned int) HAL_GetTick());
  	      xQueueSend(pPrintQueue, string, 0);
#endif
		  switch(BT1RxMsg.DU[1]) // Received BT module action
		  {
		    case 0x00: // BT module has no action
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module has no action.\n", 0);
#endif
	          break;
            }
		    case 0x01: // BT module is Idle (advertising)
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module is Idle (advertising).\n", 0);
#endif
	          break;
            }
		    case 0x02: // BT module is scanning
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module is scanning.\n", 0);
#endif
	          break;
            }
		    case 0x03: // BT module is connected, FS_BTMAC address of connected module is in more info
		    {
#if PRINTF_BT_COMMANAGER
		    	sprintf(string, ", module is connected. MAC address = 0x");
		  	  char DUString[4];
		      for (int i = 7; i > 1; i--)
		      {
		        sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
		        strcat(string, DUString);
		      }
		      strcat(string, ".\n");
		  	  xQueueSend(pPrintQueue, string, 0);
#endif
	          break;
            }
		    case 0x04: // BT module is sleeping (system-off mode)
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module is sleeping (system-off mode).\n", 0);
#endif
	          break;
            }
		    case 0x05: // BT module is in Direct test mode (DTM)
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", module is in direct test mode (DTM).\n", 0);
#endif
	          break;
            }
		    default: // Invalid BT module action value received
		    {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", invalid BT module action value received.\n", 0);
#endif
		    }
		  }
		}
		break;
		case CMD_SLEEP_CNF: //  BT module Sleep request received
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "sleep request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", will go to sleep now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DATA_CNF: //  Data transmission request received
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "data transmission request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", will send data now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_CONNECT_CNF: //  Connection setup request received
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "connection setup request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", try to connect to the device with the given MAC address now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DISCONNECT_CNF: //  Disconnection request received
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "disconnection request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", try to disconnect from the device with the given MAC address now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_SCANSTART_CNF: //  Scan started
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "scan for other modules in range request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", will start scan now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_SCANSTOP_CNF: //  Scan stopped
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "stop scanning for other modules in range request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", will stop scan now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_GETDEVICES_CNF: //  Output the scanned/detected devices
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "output of the scanned/detected devices request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
            if (!BT1RxMsg.DU[1]) // # of devices found
            {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", but no devices in range detected.\n", 0);
#endif
            }
            else
            {
              if (BT1RxMsg.DU[1] > 0x01) // # of devices found
              {
#if PRINTF_BT_COMMANAGER
              sprintf(string, ", 0x%02X devices in range detected.\n",BT1RxMsg.DU[1]);
	  	      xQueueSend(pPrintQueue, string, 0);
#endif
              }
              else
              {
#if PRINTF_BT_COMMANAGER
  	  	        xQueueSend(pPrintQueue, ", one device in range detected.\n", 0);
#endif
              }
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, "+---------------+------+---------+-------------+\n", 0);
	  	      xQueueSend(pPrintQueue, "| BTMAC address | RSSI | TXPower | Device name |\n", 0);
	  	      xQueueSend(pPrintQueue, "+---------------+------+---------+-------------+\n", 0);
#endif
              // todo print found device list
            }
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_SETBEACON_CNF:  //  Data is placed in scan response packet
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "beacon data in scan response packet request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", will place data now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		} break;
		case CMD_PASSKEY_CNF: //  Received the pass key
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "pass key request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, ", pass key accepted and pass key request answered.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DELETEBONDS_CNF: //  Deleted bonding information
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, "removing bonding information request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
	  	    xQueueSend(pPrintQueue, " and successfully processed.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_GETBONDS_CNF: //  Return the MAC of all bonded devices
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, " MAC address of all bonded devices request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
            if (!BT1RxMsg.DU[1]) // # of devices found
            {
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, ", but no bonded devices available.\n", 0);
#endif
            }
            else
            {
              if (BT1RxMsg.DU[1] > 0x01) // # of devices found
              {
#if PRINTF_BT_COMMANAGER
              sprintf(string, ", 0x%02X bonded devices available.\n",BT1RxMsg.DU[1]);
	  	      xQueueSend(pPrintQueue, string, 0);
#endif
              }
              else
              {
#if PRINTF_BT_COMMANAGER
  	  	        xQueueSend(pPrintQueue, ", one bonded device available.\n", 0);
#endif
              }
#if PRINTF_BT_COMMANAGER
	  	      xQueueSend(pPrintQueue, "+---------+---------------+\n", 0);
	  	      xQueueSend(pPrintQueue, "| Bond_ID | BTMAC address |\n", 0);
	  	      xQueueSend(pPrintQueue, "+---------+---------------+\n", 0);
#endif
              // todo print Bond_ID list
            }
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_GET_CNF: //  Return the requested module flash settings
			              //  for more info on the value of requested flash setting parameter,
			              //  see Table 17 (page 125) of Proteus II reference manual
		{
#if PRINTF_BT_COMMANAGER
  	      xQueueSend(pPrintQueue, " flash setting parameter read request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
        	sprintf(string, ", read out of setting successful.\nFlash parameter value = 0x");
		  	char DUString[3];
	        for (int i = 1; i < BT1RxMsg.length; i++)
	        {
	          sprintf(DUString, "%02X",BT1RxMsg.DU[i]);
	          strcat(string, DUString);
	        }
	        strcat(string, ".\n");
	  	    xQueueSend(pPrintQueue, string, 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_SET_CNF: //  Module flash settings have been modified
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, " flash setting parameter write request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, " and successfully processed.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_PHYUPDATE_CNF: //  Update Phy request received
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "update PHY request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, ", try to update PHY of current connection.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_UARTDISABLE_CNF: //  Disable UART request received
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "disable UART request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
            xQueueSend(pPrintQueue, ", will disable UART now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_FACTORYRESET_CNF: //  Factory reset request received
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "factory reset request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, ", will perform factory reset now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DTMSTART_CNF: //  Enable the direct test mode now
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "direct test mode (DTM) request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, ", will enable the DTM now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DTM_CNF: //  Test of direct test mode started/stopped
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "start/stop various test modes request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
            if (!BT1RxMsg.DU[1]) // Result
            {
              if (!BT1RxMsg.DU[2]) // Result
              {
#if PRINTF_BT_COMMANAGER
      	        xQueueSend(pPrintQueue, ", test is successful.\n", 0);
#endif
              }
              else
              {
#if PRINTF_BT_COMMANAGER
      	        xQueueSend(pPrintQueue, " but test failed.\n", 0);
#endif
              }
            }
            else
            {
#if PRINTF_BT_COMMANAGER
              sprintf(string, " received 0x%02X packets during RX test.\n",BT1RxMsg.DU[3]);
    	      xQueueSend(pPrintQueue, string, 0);
#endif
            }
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_BOOTLOADER_CNF: //  Will switch to bootloader now
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "reset and OTA boot loader request received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, ", will start boot loader now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_SLEEP_IND: //  State will be changed to ACTION_SLEEP
		{
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sleep indication received", 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, ", advertising timeout has expired without a connection to the module, will go to sleep now.\n", 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_DISCONNECT_IND: //  Disconnected
		{
		  sprintf(string, "disconnected");
          switch(BT1RxMsg.DU[0]) // Reason of disconnection
          {
            case 0x08: {strcat(string, " due to a connection timeout.\n");break;}
            case 0x13: {strcat(string, " by the user.\n");break;}
            case 0x16: {strcat(string, " by the host.\n");break;}
            case 0x3B: {strcat(string, ". The connection interval is unacceptable.\n");break;}
            case 0x3D: {strcat(string, " due to MIC failure. Either bad link quality or connection request ignored due to wrong key.\n");break;}
            case 0x3E: {strcat(string, " due to failure of connection setup.\n");break;}
            default:   {strcat(string, " but reason value received is unknown.\n");}
          }
#if PRINTF_BT_COMMANAGER
    	  xQueueSend(pPrintQueue, string, 0);
#endif
          // find out which module is disconnected
          // Because app_BT1_com is dedicated to Bluetooth 1, it is easy to set the connection status to 0...
    	  HAL_GPIO_WritePin(LED_GOOD_GPIO_Port, LED_GOOD_Pin, GPIO_PIN_RESET);
    	  HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
    	  imu_array[0]->connected = 0;
    	  imu_array[0]->is_calibrated = 0;
    	  imu_array[0]->measuring = 0;
    	  imu_array[0]->outputDataTypeGiven = 0;
    	  imu_array[0]->sampleRateGiven = 0;
// todo
//#if PRINTF_BT_COMMANAGER
//          sprintf(string, "%u [app_BT1_com] [BTComManagerThread] Find out which channel is disconnected.\n",(unsigned int) HAL_GetTick());
//	        xQueueSend(pPrintQueue, string, 0);
//#endif
//      	  for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++){
//      		if(imu_array[i]->connected)
//    		{
//      		  BT_transmit_CMD(imu_array[i]->uart, CMD_GETSTATE_REQ);
//      		}
//      	  }
		}
		break;
		case CMD_SECURITY_IND: //  Secured connection established
		{
#if PRINTF_BT_COMMANAGER
          switch(BT1RxMsg.DU[0]) // Reason of disconnection
          {
            case 0x00: {sprintf(string, " encrypted link to previously bonded device established.");break;}
            case 0x01: {sprintf(string, " bonding successful, encrypted link established.");break;}
            case 0x02: {sprintf(string, " no bonding, pairing successful, encrypted link established.");break;}
            default:   {sprintf(string, " security indication has unknown value.");}
          }
            strcat(string, "Connected device MAC address = 0x");
		  	char DUString[4];
            for (int i = 6; i > 0; i--)
            {
              sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
              strcat(string, DUString);
            }
            strcat(string, ".\n");
  	        xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_RSSI_IND: //  Advertising package detected
		{
#if PRINTF_BT_COMMANAGER
		  sprintf(string, " advertising package detected received from module with MAC address = 0x");
		  char DUString[4];
          for (int i = 5; i >= 0; i--)
          {
            sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
            strcat(string, DUString);
          }
	      xQueueSend(pPrintQueue, string, 0);
          sprintf(string, ", RSSI = %d dBm and TX Power %d dBm.\n",(int)BT1RxMsg.DU[6],(int)BT1RxMsg.DU[7]);
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_BEACON_IND:  //  Received Beacon data
		{
#if PRINTF_BT_COMMANAGER
		  sprintf(string, "Beacon data received from module with MAC address = 0x");
		  char DUString[4];
          for (int i = 5; i >= 0; i--)
          {
            sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
            strcat(string, DUString);
          }
	      xQueueSend(pPrintQueue, string, 0);
          sprintf(string, ", RSSI = %d dBm.\n",(int)BT1RxMsg.DU[6]);
          sprintf(string, "Payload: 0x");
          for (int i = 7; i < BT1RxMsg.length; i++)
          {
            sprintf(DUString, " %02X",BT1RxMsg.DU[i]);
            strcat(string, DUString);
          }
          strcat(string, "\n");
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_PASSKEY_IND: //  Received a pass key request
		{
#if PRINTF_BT_COMMANAGER
	      sprintf(string, "received a pass key request");
	      xQueueSend(pPrintQueue, string, 0);
#endif
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
#if PRINTF_BT_COMMANAGER
        	sprintf(string, ", with success from module with MAC address = 0x");
  		    char DUString[4];
            for (int i = 6; i > 0; i--)
            {
              sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
              strcat(string, DUString);
            }
            strcat(string, "\n");
  	        xQueueSend(pPrintQueue, string, 0);
#endif
          }
          else // Received status is not OK
          {
        	bt_RecMsgNotOK();
          }
		}
		break;
		case CMD_PHYUPDATE_IND: //  PHY has been updated
		{
#if PRINTF_BT_COMMANAGER
	      sprintf(string, "PHY attempt update done, ");
	      xQueueSend(pPrintQueue, string, 0);
          if (!BT1RxMsg.DU[0]) // Received status is OK
          {
            sprintf(string, "with success. PHY Rx = %d MBit, PHY Tx = %d MBit with module MAC address = 0x",(int)BT1RxMsg.DU[1],(int)BT1RxMsg.DU[2]);
  		    char DUString[4];
            for (int i = 8; i > 2; i--)
            {
              sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
              strcat(string, DUString);
            }
            strcat(string, ".\n");
          }
          else // Received status is not OK
          {
        	  sprintf(string, "but failed");
              if (BT1RxMsg.DU[1] == 0x1A) {strcat(string, " due to unsupported feature of remote device.\n");}
              else {strcat(string, " due to unknown reason.\n");}
          }
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_UARTENABLE_IND: //  UART was re-enabled
		{
#if PRINTF_BT_COMMANAGER
	      sprintf(string, " UART was re-enabled");
          if (!BT1RxMsg.DU[0]) {strcat(string, " successfully.\n");}
          else {strcat(string, ", but unknown status received.\n");}
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_ERROR_IND: //  Entered error state
		{
#if PRINTF_BT_COMMANAGER
		  sprintf(string, " entered error state");
	      xQueueSend(pPrintQueue, string, 0);
          if (BT1RxMsg.DU[0] == 0x01) {strcat(string, " because of a UART communication error. UART restarted. Restart module if UART is still malfunctioning.\n");}
          else {strcat(string, ", but unknown status received.\n");}
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
		break;
		case CMD_TXCOMPLETE_RSP: //  Data has been sent
		{
#if PRINTF_BT_COMMANAGER
          if (!BT1RxMsg.DU[0]) {xQueueSend(pPrintQueue, "data transmitted successfully.\n", 0);}
          else {xQueueSend(pPrintQueue, "data transmission failed.\n", 0);}
#endif
		}
		break;
		case CMD_CHANNELOPEN_RSP: //  Channel open, data transmission possible
		{
          if (!BT1RxMsg.DU[0]) // Data transmission success
          {
//            int64_t connectedMACAddress = 0;
//            int64_t ArrayMACAddress = 0;
#if PRINTF_BT_COMMANAGER
	        sprintf(string, "channel open, data transmission possible. Maximum payload is 0x%02X bytes with module MAC address = 0x",BT1RxMsg.DU[7]);
  		    char DUString[4];
            for (int i = 6; i > 0; i--)
            {
              sprintf(DUString, ".%02X",BT1RxMsg.DU[i]);
              strcat(string, DUString);
            }
            strcat(string, ".\n");
  	        xQueueSend(pPrintQueue, string, 0);
#endif
            // find out which channel is connected...
            int match = 0;
            int MACZero = 0;
            int ChannelFound = 6;
            for (int i = 0; i < 6; i++) // check for all 6 BT channels
            {
              for (int j = 0; j < 6; j++)
              {
        	    if (imu_array[i]->mac_address[j] == BT1RxMsg.DU[j+1])
        	    {
        	      match++;
        	      MACZero += imu_array[i]->mac_address[j];
        	    }
              }
              if (match == 6)
              {
            	if (MACZero != 0)
            	{
                  ChannelFound = i;
                  MACZero = 0;
            	}
              }
            }
            if (ChannelFound != 6)
            {
          	  imu_array[ChannelFound]->connected = 1;
#if PRINTF_BT_COMMANAGER
              sprintf(string, "%u [app_BT1_com] [BTComManagerThread] New open channel identified as %s and registered as connected.\n",(unsigned int) HAL_GetTick(),imu_array[ChannelFound]->name);
    	      xQueueSend(pPrintQueue, string, 0);
#endif
            }
          }
          else // Received status is not OK
          {
#if PRINTF_BT_COMMANAGER
  	        xQueueSend(pPrintQueue, " opening channel failed. Unknown received status value.\n", 0);
#endif
          }
		}
		break;
		default:
#if PRINTF_BT_COMMANAGER
	      sprintf(string, "%u [app_BT1_com] [BTComManagerThread] BT module unknown command received: 0x%02X.",(unsigned int) HAL_GetTick(),BT1RxMsg.command);
	      xQueueSend(pPrintQueue, string, 0);
#endif
		break;
	  }
	}
//	osDelay(5); // every 200 HAL Ticks
  }
}

void bt_RxHandler(char c)
{
    // Queue in ring buffer the rx'd byte
    ring_buffer_queue(&BT1RingbufRx, c);
}

void bt_init()
{
    ring_buffer_init(&BT1RingbufRx);
    bt_rst_msg(&BT1RxMsg);
}

void bt_rst_msg(BT_msg_t * BTMsg)
{
  if (BTMsg != NULL)
  {
    BTMsg->command = 0;
    BTMsg->length = 0;
    BTMsg->CS = 0;
    memset(BTMsg->DU, 0, MAX_BT_PAYLOAD_LENGTH);
  }
}

BT_DECODER_RESULT bt_prot_decoder(BT_msg_t * BTMsg)
{
  static BT_PARSER_STATE BTState = BT_IDLE;
  BT_DECODER_RESULT BTRes = BT_NO_MSG;
  //static unsigned int timeout = 0;

  switch (BTState)
  {
    case BT_IDLE: // check if start byte is found
    {
      if (bt_FindSdByte())
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_IDLE] found BT Start Delimiter.\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
//        BT_RstTimeout(&timeout);
//        BT_rst_msg(&tmpBTMsg);
        BTState = BT_BUILDING_HEADER;
      }
      else
      {
        BTRes = BT_NO_MSG;
        break;
      }
    }
    /* no break */
    case  BT_BUILDING_HEADER: // check if command and length is available
    {

#if PRINTF_BT_COM
      sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Start building header.\n",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
#endif
      if (bt_HeaderPartPresent())
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] BT header part present.\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
//        BT_RstTimeout(&timeout);
        if (bt_BuildHeader())
        {
          /* Could build a header */
#if PRINTF_BT_COM
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Command received is 0x%02X\n",(unsigned int) HAL_GetTick(), BT1RxMsg.command);
	      xQueueSend(pPrintQueue, string, 0);
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Length of body is %d\n",(unsigned int) HAL_GetTick(), BT1RxMsg.length);
	      xQueueSend(pPrintQueue, string, 0);
#endif
          BTState = BT_BUILDING_BODY;
          BTRes = BT_IN_PROGRESS;
        }
        else
        {
          /* Invalid header */
#if PRINTF_BT_COM
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Less than 3 elements in the buffer\n",(unsigned int) HAL_GetTick());
	      xQueueSend(pPrintQueue, string, 0);
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Back to Idle, length in buffer is %d\n",(unsigned int) HAL_GetTick(), BT1RxMsg.length);
	      xQueueSend(pPrintQueue, string, 0);
#endif
          BTState = BT_IDLE;
          BTRes = BT_NO_MSG;
          break;
        }
      }
      else
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Less then 3 items in buffer\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
//        if (BT_TimedOut(&timeout))
//        {
//#if PRINTF_BT_COM
//          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_HEADER] Time out in building header, back to CPL_IDLE state. Timeout =  %d\n",(unsigned int) HAL_GetTick(), timeout);
//	        xQueueSend(pPrintQueue, string, 0);
//#endif
//          BTState = BT_IDLE;
//          break;
//        }
      }
    }
    /* no break */
    case  BT_BUILDING_BODY: // check if payload is available (length bytes)
    {

#if PRINTF_BT_COM
      sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] start building body.\n",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
#endif
      if (bt_BodyPartPresent())
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] [BT_BodyPartPresent] %u elements in ringbuffer, >= %d length.\n",(unsigned int) HAL_GetTick(),(unsigned int) ring_buffer_num_items(&BT1RingbufRx),(BT1RxMsg.length+5));
	    xQueueSend(pPrintQueue, string, 0);
#endif
        if (bt_BuildBody())
        {
#if PRINTF_BT_COM
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] Going to check frame\n",(unsigned int) HAL_GetTick());
	      xQueueSend(pPrintQueue, string, 0);
#endif
          BTState = BT_CHECKING_FRAME;
          BTRes = BT_IN_PROGRESS;
        }
        else
        {
#if PRINTF_BT_COM
          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] Back to Idle\n",(unsigned int) HAL_GetTick());
	      xQueueSend(pPrintQueue, string, 0);
#endif
          BTState = BT_IDLE;
//          if (BT_TimedOut(&timeout))
//          {
//            BTState = CPL_IDLE;
//#if PRINTF_BT_COM
//            sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] Time out in building body. smState = CPL_IDLE.\n",(unsigned int) HAL_GetTick());
//	          xQueueSend(pPrintQueue, string, 0);
//#endif
//            break;
//          }
          BTRes = BT_NO_MSG;
          break;
        }
//        BT_RstTimeout(&timeout);
      }
      else
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] BT body part not present: expected %d elements, but only %u elements available.\n",(unsigned int) HAL_GetTick(),(BT1RxMsg.length+5),(unsigned int) ring_buffer_num_items(&BT1RingbufRx));
	    xQueueSend(pPrintQueue, string, 0);
#endif
//        if (BT_TimedOut(&timeout))
//        {
//          BTState = CPL_IDLE;
//#if PRINTF_BT_COM
//          sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_BUILDING_BODY] Time out in building body, smState = CPL_IDLE.\n",(unsigned int) HAL_GetTick());
//	        xQueueSend(pPrintQueue, string, 0);
//#endif
//          break;
//        }
        break;
      }
    }
    /* no break */
    case  BT_CHECKING_FRAME: // calculate checksum and compare
    {
#if PRINTF_BT_COM
      sprintf(string, "%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_CHECKING_FRAME] BT receiving buffer: ",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
      sprintf(string, "0x02 0x%02X 0x%04X",BT1RxMsg.command,BT1RxMsg.length);
      xQueueSend(pPrintQueue, string, 0);
      for (int i = 0; i < (BT1RxMsg.length); i++)
      {
        sprintf(string, " 0x%02X",BT1RxMsg.DU[i]);
	    xQueueSend(pPrintQueue, string, 0);
      }
      sprintf(string, " 0x%02X\n",BT1RxMsg.CS);
      xQueueSend(pPrintQueue, string, 0);
#endif
      if (bt_csCorrect())
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_CHECKING_FRAME] Check Sum correct.\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
        BTRes = BT_CORRECT_FRAME;
        BTState = BT_IDLE;
        break;
      }
      else
      {
#if PRINTF_BT_COM
        sprintf(string,"%u [app_BT1_com] [bt_prot_decoder] [BTState = BT_CHECKING_FRAME] Check Sum not correct, back to Idle.\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
        BTState = BT_IDLE;
        BTRes = BT_INCORRECT_FRAME;
      }
    }
  }
  return BTRes;
}

static int bt_FindSdByte(void)
{
  char rxdByte;
  rxdByte = 0;
  while (!ring_buffer_is_empty(&BT1RingbufRx))
  {
	ring_buffer_peek(&BT1RingbufRx, &rxdByte, 0);
    if (rxdByte == BT_START_DELIMITER)
    {
      return 1;
    }
    else
    {
      ring_buffer_dequeue(&BT1RingbufRx, &rxdByte);
    }
  }
  return 0;
}
static int bt_HeaderPartPresent(void)
{
  if (ring_buffer_num_items(&BT1RingbufRx) >= 4) // start delimiter + command byte + 2 bytes representing length of body
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

static int bt_BuildHeader(void)
{
  char rxdByte[3];
  memset(rxdByte, 0, 3);
  if (ring_buffer_num_items(&BT1RingbufRx) >= 4)
  {
    ring_buffer_peek(&BT1RingbufRx, &rxdByte[0], 1);
    ring_buffer_peek(&BT1RingbufRx, &rxdByte[1], 2);
    ring_buffer_peek(&BT1RingbufRx, &rxdByte[2], 3);
    BT1RxMsg.command = rxdByte[0];
    BT1RxMsg.length = (int) rxdByte[1] + 256 * (int) rxdByte[2];
#if PRINTF_BT_COM
    sprintf(string, "[app_BT_com][bt_BuildHeader] header: 0x%02X 0x%04X\n",BT1RxMsg.command,BT1RxMsg.length);
    xQueueSend(pPrintQueue, string, 0);
#endif
    return 1;
  }
  else
  {
    /* Less than 3 elems in the buffer */
	return 0;
  }
}

static int bt_BodyPartPresent(void)
{
  if (ring_buffer_num_items(&BT1RingbufRx) >= (BT1RxMsg.length +5))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

static int bt_BuildBody(void)
{
  for (unsigned int i = 0; i < (BT1RxMsg.length); i++)
  {
    if (!ring_buffer_peek(&BT1RingbufRx, (char*)&BT1RxMsg.DU[i], i+4))
    {
      /* No item at index... */
#if PRINTF_BT_COM
      sprintf(string,"%u [app_BT1_com] [bt_BuildBody] ring_buffer_peek(), no item at index %02X\n",(unsigned int) HAL_GetTick(), BT1RxMsg.DU[i]);
      xQueueSend(pPrintQueue, string, 0);
#endif
      return 0;
    }
    ring_buffer_peek(&BT1RingbufRx, (char*)&BT1RxMsg.CS, BT1RxMsg.length+4);
  }
#if PRINTF_BT_COM
  sprintf(string,"%u [app_BT1_com] [bt_BuildBody] Data Units of ring_buffer correctly transferred to BT1RxMsg.DU\n",(unsigned int) HAL_GetTick());
  xQueueSend(pPrintQueue, string, 0);
#endif
  return 1;
}

static int bt_csCorrect(void)
{
  char rxdByte[2];
  char CScalc;
  memset(rxdByte, 0, 2);
//  ring_buffer_peek(&BT1RingbufRx, &rxdByte[0], BT1RxMsg.length+5);
  ring_buffer_peek(&BT1RingbufRx, &rxdByte[0], 2);
  ring_buffer_peek(&BT1RingbufRx, &rxdByte[1], 3);
//  BT1RxMsg.CS = rxdByte[0];
  CScalc = 0x02 ^ BT1RxMsg.command ^ rxdByte[0] ^ rxdByte[1];
  for (unsigned int i = 0; i < (BT1RxMsg.length); i++)
  {
    CScalc = CScalc ^ BT1RxMsg.DU[i];
  }
  if (CScalc == BT1RxMsg.CS)
  {
	ring_buffer_dequeue_arr(&BT1RingbufRx, NULL, BT1RxMsg.length+5);
	return 1;
  }
  else
  {
    return 0;
  }
}

static int bt_RecMsgNotOK(void)
{
#if PRINTF_BT_COMMANAGER
  switch(BT1RxMsg.DU[0]) // Received Status
  {
    case 0x01: {xQueueSend(pPrintQueue, " but operation failed.\n", 0);break;}
    case 0x03: {xQueueSend(pPrintQueue, " but device busy.\n", 0);break;}
    case 0x04: {xQueueSend(pPrintQueue, " but serious error when writing to flash. Try to factory reset or re-flash the device.\n", 0);break;}
    case 0xFF: {xQueueSend(pPrintQueue, " but operation not permitted.\n", 0);}
    default: {xQueueSend(pPrintQueue, " but received status is invalid.\n", 0);}
  }
#endif
  return 1;
}


void rsv_data_msg_handler(void)
{
#if PRINTF_BT_COMMANAGER
  sprintf(string, "%u [app_BT1_com] [rsv_data_msg_handler] %s ", (unsigned int) HAL_GetTick(), imu_array[BTChannel]->name);
  xQueueSend(pPrintQueue, string, 0);
#endif
  switch(BT1RxMsg.DU[7])
  {
    case IMU_SENSOR_MODULE_IND_BATTERY_VOLTAGE:
    {
      imu_array[BTChannel]->battery_voltage = (BT1RxMsg.DU[8] | BT1RxMsg.DU[9] << 8);
	  float voltage = (BT1RxMsg.DU[8] | BT1RxMsg.DU[9] << 8)/100.0;
#if PRINTF_BT_COMMANAGER
	  sprintf(string, "battery voltage = %.02fV.\n", voltage);
      xQueueSend(pPrintQueue, string, 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_BATTERY_LOW_ERROR:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "battery voltage is too low, re-charge the battery.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SYNC_STARTED:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "synchronization started.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SYNC_DONE:{
	  imu_array[BTChannel]->sync_time = HAL_GetTick();
#if PRINTF_BT_COMMANAGER
	  sprintf(string, "synchronization done. DCU System Tick = %d.\n", (int)imu_array[BTChannel]->sync_time);
      xQueueSend(pPrintQueue, string, 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_CANNOT_SYNC:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "can not be synchronized at this moment.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_NEED_TO_SYNCHRONISE:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "needs to synchronize before a measurement can be started.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SYNC_TIME:
	{
	  imu_array[BTChannel]->sync_time = HAL_GetTick();
#if PRINTF_BT_COMMANAGER
	  sprintf(string, "DCU System Tick = %d.\n",(int)imu_array[BTChannel]->sync_time);
      xQueueSend(pPrintQueue, string, 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SYNC_TIME_CHANGED:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "synchronization time changed.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SLEEP_MODE:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "is going in sleep mode.\n", 0);
#endif
	  HAL_GPIO_WritePin(LED_GOOD_GPIO_Port, LED_GOOD_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
      imu_array[BTChannel]->measuring = 0;
	  imu_array[BTChannel]->connected = 0;
	  imu_array[BTChannel]->is_calibrated = 0;
	  imu_array[BTChannel]->sampleRateGiven = 0;
	  imu_array[BTChannel]->outputDataTypeGiven = 0;
	  break;
	}
	case IMU_SENSOR_MODULE_IND_CANNOT_CALIBRATE:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "calibration not possible at this moment.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_CALIBRATION_STARTED:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "calibration started.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_CALIBRATION_DONE:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "calibration done.\n", 0);
#endif
	  imu_array[BTChannel]->is_calibrated = 1;
	  break;
	}
	case IMU_SENSOR_MODULE_IND_NEED_TO_CALIBRATE:{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "calibration is needed before the measurement can start.\n", 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_MEASUREMENTS_STOPPED:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "stopped measuring.\n", 0);
#endif
	  HAL_GPIO_WritePin(LED_GOOD_GPIO_Port, LED_GOOD_Pin, GPIO_PIN_RESET);
	  HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_SET);
	  imu_array[BTChannel]->measuring = 0;
	  break;
	}
	case IMU_SENSOR_MODULE_IND_MEASUREMENTS_STARTED:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "measurement started.\n", 0);
#endif
	  imu_array[BTChannel]->measuring = 1;
	  break;
	}
	case IMU_SENSOR_MODULE_IND_MEASUREMENTS_STARTED_WITHOUT_SYNC:
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "measurement started without synchronization.\n", 0);
#endif
	  imu_array[BTChannel]->measuring = 1;
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_SEND_DATA_F1: //  QUAT
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "receiving measurement data, quaternions only.\n", 0);
#endif
	  //rsv_data_handler(rsvbuf, imu->number, DATA_FORMAT_1);
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_SEND_DATA_F2: //  GYRO
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "receiving measurement data, gyroscope only.\n", 0);
#endif
	  //rsv_data_handler(rsvbuf, imu->number, DATA_FORMAT_2);
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_SEND_DATA_F3: //	ACC
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "receiving measurement data, accelerometer only.\n", 0);
#endif
	  //rsv_data_handler(rsvbuf, imu->number, DATA_FORMAT_3);
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_SEND_DATA_F4: //	GYRO + ACC
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "receiving measurement data, gyroscope and accelerometer.\n", 0);
#endif
	  //rsv_data_handler(rsvbuf, imu->number, DATA_FORMAT_4);
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_SEND_DATA_F5: //	QUAT + GYRO + ACC
	{
#if PRINTF_BT_COMMANAGER
      xQueueSend(pPrintQueue, "receiving measurement data, quaternions, gyroscope and accelerometer.\n", 0);
#endif
	  //rsv_data_handler(rsvbuf, imu->number, DATA_FORMAT_5);
	  break;
	}
	case IMU_SENSOR_MODULE_RSP_MILLIS:
	{
	  uint32_t millis = (((BT1RxMsg.DU[8] | BT1RxMsg.DU[9] << 8) | BT1RxMsg.DU[10] << 16) | BT1RxMsg.DU[11] << 24);
#if PRINTF_BT_COMMANAGER
	  sprintf(string, "system Tick = %d.\n", (int)millis);
      xQueueSend(pPrintQueue, string, 0);
#endif
	 break;
	}
	case IMU_SENSOR_MODULE_RSP_SW_VERSION:
	{
	  float software_version = BT1RxMsg.DU[8]/10.0;
#if PRINTF_BT_COMMANAGER
	  sprintf(string, "sensor module software version = %.01f.\n", software_version);
      xQueueSend(pPrintQueue, string, 0);
#endif
	  break;
	}
	case IMU_SENSOR_MODULE_IND_SAMPLING_FREQ_CHANGED:
	{
	  switch(BT1RxMsg.DU[8])
	  {
		case SAMPLING_FREQ_10HZ:
		{
		  imu_array[BTChannel]->sampleRateGiven = 1;
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sampling rate changed to 10Hz.\n", 0);
#endif
          break;
		}
		case SAMPLING_FREQ_20HZ:
		{
		  imu_array[BTChannel]->sampleRateGiven = 1;
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sampling rate changed to 20Hz.\n", 0);
#endif
          break;
		}
		case SAMPLING_FREQ_25HZ:
		{
		  imu_array[BTChannel]->sampleRateGiven = 1;
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sampling rate changed to 25Hz.\n", 0);
#endif
          break;
		}
		case SAMPLING_FREQ_50HZ:
		{
		  imu_array[BTChannel]->sampleRateGiven = 1;
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sampling rate changed to 50Hz.\n", 0);
#endif
          break;
		}
		case SAMPLING_FREQ_100HZ:
		{
		  imu_array[BTChannel]->sampleRateGiven = 1;
#if PRINTF_BT_COMMANAGER
	      xQueueSend(pPrintQueue, "sampling rate changed to 100Hz.\n", 0);
#endif
          break;
		}
		default:
		{
		  imu_array[BTChannel]->sampleRateGiven = 0;
#if PRINTF_BT_COMMANAGER
	      sprintf(string, "sampling rate returned value unknown: 0x%2X.\n",BT1RxMsg.DU[8]);
	      xQueueSend(pPrintQueue, string, 0);
#endif
		}
	  }
	  break;
	}
	case IMU_SENSOR_MODULE_IND_DF_CHANGED:
	{
	  switch(BT1RxMsg.DU[8])
	  {
	    case 1:
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 1;
#if PRINTF_BT_COMMANAGER
		  xQueueSend(pPrintQueue, "data output type changed to quaternions only.\n", 0);
#endif
	      break;
		}
	    case 2: // not supported yet in RAW file definition
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 1;
#if PRINTF_BT_COMMANAGER
		  xQueueSend(pPrintQueue, "data output type changed to gyroscope only.\n", 0);
#endif
	      break;
		}
	    case 3: // not supported yet in RAW file definition
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 1;
#if PRINTF_BT_COMMANAGER
		  xQueueSend(pPrintQueue, "data output type changed to accelerometer only.\n", 0);
#endif
	      break;
		}
	    case 4: // not supported yet in RAW file definition
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 1;
#if PRINTF_BT_COMMANAGER
		  xQueueSend(pPrintQueue, "data output type changed to gyroscope and accelerometer.\n", 0);
#endif
	      break;
		}
	    case 5:
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 1;
#if PRINTF_BT_COMMANAGER
		  xQueueSend(pPrintQueue, "data output type changed to quaternions, gyroscope and accelerometer.\n", 0);
#endif
	      break;
		}
		default:
		{
		  imu_array[BTChannel]->outputDataTypeGiven = 0;
#if PRINTF_BT_COMMANAGER
	      sprintf(string, "data output type returned value unknown: 0x%2X.\n",BT1RxMsg.DU[8]);
	      xQueueSend(pPrintQueue, string, 0);
#endif
	      break;
		}
	  }
	break;
	}
	case IMU_SENSOR_MODULE_IND_STATUS:
	{
	  enum Sensor_Reader_State{SLEEP, STARTUP, WAIT_FOR_CONNECTION, CALIBRATION, IDLE, SYNC, INIT_MES, RUNNING, CHARGING, BATTERY_LOW};
	  switch(BT1RxMsg.DU[8])
	  {
		case SLEEP: USB_COM_print_info(imu_array[BTChannel]->name, "SLEEP-MODE"); break;
		case STARTUP: USB_COM_print_info(imu_array[BTChannel]->name, "STARTUP-MODE"); break;
		case WAIT_FOR_CONNECTION: USB_COM_print_info(imu_array[BTChannel]->name, "WAIT_FOR_CONNECTION-MODE"); break;
		case CALIBRATION: USB_COM_print_info(imu_array[BTChannel]->name, "CALIBRATION-MODE"); break;
		case IDLE: USB_COM_print_info(imu_array[BTChannel]->name, "IDLE-MODE"); break;
		case SYNC: USB_COM_print_info(imu_array[BTChannel]->name, "SYNC-MODE"); break;
		case INIT_MES: USB_COM_print_info(imu_array[BTChannel]->name, "INIT_MES-MODE"); break;
		case RUNNING: USB_COM_print_info(imu_array[BTChannel]->name, "MEASURING-MODE"); break;
		case BATTERY_LOW: USB_COM_print_info(imu_array[BTChannel]->name, "BATTERY_LOW-MODE"); break;
		default:{}
		break;
	  }
	  break;
	}
	default:
	{
	}
  }
}

void rsv_data_handler(uint8_t * buf, uint8_t sensor_number, uint8_t data_format){

	int16_t sd_card_buffer [100];
	int16_t data [10];
	uint16_t data_values;
	uint32_t timestamp;
	uint16_t pakket_send_nr;

	switch(data_format){
		case DATA_FORMAT_1: {	data_values = 4;		} break;
		case DATA_FORMAT_2: {	data_values = 3;		} break;
		case DATA_FORMAT_3: {	data_values = 3;		} break;
		case DATA_FORMAT_4: {	data_values = 6;		} break;
		case DATA_FORMAT_5: {	data_values = 10;		} break;
	}

	//int16_t sd_card_buffer [NUMBER_OF_DATA_READS_IN_BT_PACKET * data_values];


  for(uint8_t j = 0; j < NUMBER_OF_BT_PACKETS; j++){
    uint16_t start_pos = PACKET_START_POS + 7;
		// 1e byte: 			command "IMU_SENSOR_MODULE_REQ_SEND_DATA"
		// 2e & 3e byte:		packet_send_nr
		// 3e 4e 5e 6e byte:	timestamp

		pakket_send_nr = buf[PACKET_START_POS + 1] | (buf[PACKET_START_POS + 2] << 8);
		timestamp = buf[PACKET_START_POS + 3] | (buf[PACKET_START_POS + 4] << 8) | (buf[PACKET_START_POS + 5] << 16) | (buf[PACKET_START_POS + 6] << 24);

    for(int i = 0; i < NUMBER_OF_DATA_READS_IN_BT_PACKET; i++){
      //int16_t data [data_values];
			for(uint8_t g = 0; g < data_values; g++){
				data[g] = ((buf[start_pos + g*2 + data_values*2 * i] << 8) | buf[start_pos + g*2 + 1 + data_values*2 * i]);
			}

			for(uint8_t k = 0; k < data_values; k++){
				sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_BT_PACKET + i*data_values + k] = data[k];
			}
    }
  }

	SD_CARD_COM_save_data(pakket_send_nr, timestamp, sensor_number, sd_card_buffer, data_values, data_format);
	//SD_CARD_COM_save_data_qga(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
	//SD_CARD_COM_save_data_q(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
}
