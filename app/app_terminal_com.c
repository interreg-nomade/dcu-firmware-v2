/*
 * app_terminal_com.c
 *
 *  Created on: 12 Sep 2020
 *      Author: sarah
 */

#include "app_terminal_com.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usart.h"
#include "../lib/ring_buffer/ringbuffer_char.h"
#include <string.h>
#include "../Inc/usb_com.h"
#include "../pinterface/interface_sd.h"
#include "../Inc/imu_com.h"
#include "app_rtc.h"
#include "../Inc/nRF52_driver.h"
#include "app_nRF52_com.h"
#include "common.h"

#define PRINTF_TERMINAL_COM 1

/* RTOS Variables */
static osThreadId comRxTaskHandle;

/* Com link protocol variables */
static com_msg_t ComRxMsg;
static ring_buffer_t comRingbufRx;

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;
extern imu_module imu_7;
extern imu_module imu_8;


extern imu_module *imu_array [];

extern dcu_conn_dev_t mac_addr[8];

extern Status_SD SD_Status;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint8_t previous_command = 0;
imu_module *imu = NULL;
uint8_t previous_connected_modules2 [6];
uint8_t sync_enable;

char command2;

void terminalComManagerThread(const void *params);


// ================================================================
// ===                    Static functions                      ===
// ================================================================

static uint8_t convert_ASCII_to_HEX(uint8_t msn, uint8_t lsn);
static int com_Build(void);
void com_init_rx_task(void)
{
    osDelay(100);
    /* Init the decoder */
    terminal_com_init();

#if PRINTF_TERMINAL_COM
    sprintf(string, "terminal_com_init_rx_task done. \n");
    HAL_UART_Transmit(&huart7, (uint8_t *)string, strlen(string), 25);
#endif

    memset(&ComRxMsg, 0, sizeof(com_msg_t));

    osThreadDef(terminalComManager, terminalComManagerThread, osPriorityRealtime, 0, 1024);
    comRxTaskHandle = osThreadCreate(osThread(terminalComManager), NULL);
}

void terminalComManagerThread(const void *params)
{
	/* Initialize the local container */
	osDelay(100);
	terminal_com_init();

	for(;;)
	{
		if (com_Build())
		{
//#if PRINTF_TERMINAL_COM
//			sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Data received via Terminal: 0x%2X\n",(unsigned int) HAL_GetTick(),command);
//    	      xQueueSend(pPrintQueue, string, 0);
//#endif
			if(command2 == 0x0A) return;
			if(previous_command != 0)
			{
				switch(command2)
				{
					case 0x31: // "1"
					{
						//imu = &imu_1;
						if (0x66 == previous_command)
						{
							//send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_10HZ);
						}
					} break;
					case 0x32: // "2"
					{
						//imu = &imu_2;
						if (0x66 == previous_command)
						{
							//send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_20HZ);
						}
					} break;
					case 0x33: // "3"
					{
						//imu = &imu_3;
						if(0x66 == previous_command)
						{
							//send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_25HZ);
						}
					} break;
					case 0x34: // "4"
					{
						//imu = &imu_4;
						if(0x66 == previous_command)
						{
							//send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_50HZ);
						}
					} break;
					case 0x35: // "5"
					{
						//imu = &imu_5;
						if(0x66 == previous_command)
						{
							//send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_100HZ);
						}
					} break;
					case 0x36: // "6"
					{
						//imu = &imu_6;
					} break;
					default:
						imu = NULL;
						sprintf(string, "Please enter a number between 1 and 6!\n");
			            xQueueSend(pPrintQueue, string, 0);
				}
				command2 = previous_command;
				previous_command = 0;
			}
			switch(command2)
			{
				case 0x30:  // "0": Print MENU
				{
					USB_COM_show_menu();
				}
				break;
				case 0x31: // "1": Send connected device list to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Send connected device list to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_set_mac_addr(mac_addr,sizeof(mac_addr));
//					if(imu != NULL)
//					{
//						sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Connect IMU 0x%1X with MAC Address 0x%2X.\n",(unsigned int) HAL_GetTick(),imu->number,*imu->mac_address);
//			      	    xQueueSend(pPrintQueue, string, 0);
//						IMU_connect(imu);
//						imu = NULL;
//					}
//					else
//					{
//						previous_command = 0x31;
//						sprintf(string, "IMU number: \n");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
				}
				break;
				case 0x32:  // "2": Request connected device list to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request connected device list to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
		      	    comm_req_conn_dev();
//					if(imu != NULL)
//					{
//						IMU_disconnect(imu);
//						imu = NULL;
//					}
//					else
//					{
//						previous_command = 0x32;
//						sprintf(string, "IMU number: ");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
				}
				break;
				case 0x33: // "3" Request to start measurement to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request to start measurement to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_start_meas();
//					if(imu != NULL)
//					{
//						IMU_go_to_sleep(imu);
//						imu = NULL;
//					}
//					else
//					{
//						previous_command = 0x33;
//						sprintf(string, "IMU number: ");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
////					for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
////					{
////						if(imu_array[i]->connected)	IMU_go_to_sleep(imu_array[i]);
////					}
				}
				break;
				case 0x34:  //"4": Request to stop measurement to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request to stop measurement to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
		      	    comm_stop_meas();
//					for(uint8_t i = 0; i < 6; i++)
//					{
//						if(imu_array[i]->connected)	IMU_get_battery_voltage(imu_array[i]);
//					}
					//IMU_get_battery_voltage(&imu_1);
				}
				break;
				case 0x35:  //"5": Send output data type for all modules to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Send output data type for all modules to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_set_data_type(COMM_CMD_MEAS_QUAT6);
					//compare_RTCunixtime_Epoch();
					//for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
					//{
					//	if(imu_array[i]->connected)	IMU_get_systicks(imu_array[i]);
					//}
				}
				break;
				case 0x36:  //"6": Request to start synchronization to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request to start synchronization to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_set_sync(COMM_CMD_START_SYNC);
				}
				break;
				case 0x37:  //"7": Request to stop synchronization to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request to stop synchronization to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_set_sync(COMM_CMD_STOP_SYNC);
				}
				break;
				case 0x38:  //"8": Request calibration of connected modules to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request calibration of connected modules to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_calibrate();
					//IMU_synchronisation_adaptation(*imu_array);
				}
				break;
				case 0x39:  //"9": Reset nRF52 - not implemented
				{
//					if(imu != NULL)
//					{
//						read_mac_address(imu);
//						imu = NULL;
//					}
//					else
//					{
//						previous_command = 0x39;
//						sprintf(string, "Slot number: ");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
				}
				break;
				case 0x61: // "a": Request battery level of connected modules to nRF52
				{
#if PRINTF_TERMINAL_COM
				    sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Request battery level of connected modules to nRF52.\n",(unsigned int) HAL_GetTick());
		      	    xQueueSend(pPrintQueue, string, 0);
#endif
					comm_req_batt_lvl();
					//send_to_all_sensors(IMU_get_status, *imu_array);
				}
				break;
//				case 0x63:  // "c": Calibration
//				{
////					for(uint8_t i = 0; i < 6; i++)
////					{
////						if(imu_array[i]->connected)	IMU_start_calibration(imu_array[i]);
////					}
//					//IMU_start_calibration(&imu_1);
////					send_to_all_sensors(IMU_start_calibration, *imu_array);
//				}
//				break;
//				case 0x64:  // "d": get time & date
//				{
//					app_rtc_print_RTCdateTime();
//				}
//				break;
//				case 0x6E: // "n": SD card function - Create new file
//				{
//					if (!is_measuring())
//					{
//						SD_CARD_COM_create_new_file();
//					}
//					else
//					{
//						sprintf(string, "Measurement ongoing. End measurement first before generating a new file.\n");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
//				}
//				break;
				case 0x6D: // "m": show overview of module library
				{
				  module_status_overview();
				}
				break;
//				case 0x75: // "u": SD card function - Unmount SD card
//				{
//					if (!is_measuring())
//					{
//						SD_CARD_COM_unmount();
//					}
//					else
//					{
//						sprintf(string, "Measurement ongoing. End measurement first before un-mounting the SD Card.\n");
//			      	    xQueueSend(pPrintQueue, string, 0);
//					}
//				}
//				break;
				case 0x66: // "f": Change sampling frequency
				{
					comm_set_frequency(50);
				}
				case 0x0A: //"<ENTER>"
				{
					// It's just an enter (captured)
				}
				break;
				default:
					sprintf(string, "Undefined command.\n");
		      	    xQueueSend(pPrintQueue, string, 0);
			}
		}
		osDelay(4); // every 100 HAL Ticks
	}
}

void com_RxHandler(char c)
{
    // Queue in ring buffer the rx'd byte
    ring_buffer_queue(&comRingbufRx, c);
}

void terminal_com_init()
{
    ring_buffer_init(&comRingbufRx);
    com_rst_msg(&ComRxMsg);
}

static int com_Build()
{
    char rxdByte;
	if (!ring_buffer_is_empty(&comRingbufRx))
    {
//#if PRINTF_TERMINAL_COM
//		sprintf(string, "%u [app_terminal_com] [com_Build] Terminal ring buffer not empty.\n",(unsigned int) HAL_GetTick());
//  	    xQueueSend(pPrintQueue, string, 0);
//#endif
		if (!ring_buffer_is_empty(&comRingbufRx))
	    {
	        ring_buffer_dequeue(&comRingbufRx, &rxdByte);
//#if PRINTF_TERMINAL_COM
//		sprintf(string, "%u [app_terminal_com] [com_Build] Received byte: 0x%2X %c\n",(unsigned int) HAL_GetTick(),rxdByte,rxdByte);
//  	    xQueueSend(pPrintQueue, string, 0);
//#endif
	    }
	    command2 = rxdByte;
        return 1;
    }
    return 0;
}

void com_rst_msg(com_msg_t * pComMsg)
{
    if (pComMsg != NULL)
    {
        pComMsg->lenght = 0;
        memset(pComMsg->DU, 0, MAX_COM_PAYLOAD_LENGTH);
    }
}


// ================================================================
// ===                    Static functions                      ===
// ================================================================


uint8_t convert_ASCII_to_HEX(uint8_t msn, uint8_t lsn)
{
	uint8_t a = 0;
	if (msn < 0x40)
	{
		a = (msn - 0x30) << 4;
	}
	else
	{
		switch(msn)
		{
			case 0x41:	a = 0xA << 4; break;
			case 0x42:	a = 0xB << 4; break;
			case 0x43:	a = 0xC << 4; break;
			case 0x44:	a = 0xD << 4; break;
			case 0x45:	a = 0xE << 4; break;
			case 0x46:	a = 0xF << 4; break;
		}
	}
	if(lsn < 0x40)
	{
		a = a | (lsn - 0x30);
	}
	else
	{
		switch(lsn)
		{
			case 0x41:	a = a | 0xA; break;
			case 0x42:	a = a | 0xB; break;
			case 0x43:	a = a | 0xC; break;
			case 0x44:	a = a | 0xD; break;
			case 0x45:	a = a | 0xE; break;
			case 0x46:	a = a | 0xF; break;
		}
	}
	return a;
}

void module_status_overview(void)
{
  // print imu_array:
  char DUString[3];
  xQueueSend(pPrintQueue, "    Module   | Instr | ConSq |  Mac_address | Connected | Sample f | SR given | Cal | meas | bat (V) | sync time |                   output data type", 0);
  xQueueSend(pPrintQueue, "                   | ODT given\n", 0);
  xQueueSend(pPrintQueue, "-------------+-------+-------+--------------+-----------+----------+----------+-----+------+---------+-----------+-----------------------------------", 0);
  xQueueSend(pPrintQueue, "-------------------+----------\n", 0);
  // example:              BLE module 1 | C6.D8.42.14.35.E8 |     no    |    50Hz  |     no   |  no |  no  |    ?    |    ?      | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |     no
  for (int i = 0; i < 8; i++)
  {
	if (imu_array[i]->macAddressAvailable)
	{
	  sprintf(string, "%s |   %02X  |   %02X  | ", imu_array[i]->name, imu_array[i]->instrument, imu_array[i]->connectingSequence);
      for (int j = 5; j >= 0; j--)
      {
        if (strlen(string) < 147)
        {
  	   	  sprintf(DUString, "%02X",imu_array[i]->mac_address[j]);
  	   	  strcat(string, DUString);
        }
      }
      xQueueSend(pPrintQueue, string, 0);
      if (imu_array[i]->connected)
      {//                 |     no    |    50Hz  |
  	    sprintf(string, " |    yes    |  %3.0dHz   |", imu_array[i]->sampleFrequency);
      }
      else
      {
        sprintf(string, " |     no    |  %3.0dHz   |", imu_array[i]->sampleFrequency);
      }
      if (imu_array[i]->sampleRateGiven)
      {//                       |     no   |
        strcat(string, "    yes   |");
      }
      else
      {
        strcat(string, "    no    |");
      }
      if (imu_array[i]->is_calibrated)
      {//                       |  no |
        strcat(string, " yes |");
      }
      else
      {
        strcat(string, "  no |");
      }
      if (imu_array[i]->measuring)
      {//                       |  no  |
        strcat(string, "  yes |");
      }
      else
      {
        strcat(string, "  no  |");
      }
      xQueueSend(pPrintQueue, string, 0);
      //                      |    ?    |
      xQueueSend(pPrintQueue, "   xxx V |", 0); // todo battery voltage
      //                      |    ?      |
      xQueueSend(pPrintQueue, "   xxxx    |", 0); // todo sync time
      //                      | IMU Quaternions 9DOF (Quaternions only) |
      switch(imu_array[i]->outputDataType)
      {
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "        IMU Quaternions + Battery voltage level       |", 0);
          break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "                 IMU Quaternions only                 |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "      IMU Quaternions + Gyroscope + Accelerometer     |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "  IMU Quaternions + Gyroscope + Accelerometer @ 100Hz |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_100HZ:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "      IMU Quaternions (Quaternions only) @ 100Hz      |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "        IMU Quaternions 9DOF (Quaternions only)       |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF_100HZ:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "    IMU Quaternions 9DOF (Quaternions only) @ 100Hz   |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "     IMU Gyroscope + Accelerometer + Magnetometer     |", 0);
		    break;
        }
        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG_100HZ:
        {//                         | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, " IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |", 0);
		    break;
        }
		  default:
		  {//                       | IMU Gyroscope + Accelerometer + Magnetometer @ 100Hz |
		    xQueueSend(pPrintQueue, "            Unknown Data Type Output value            |", 0);
 		  }
      }
      if (imu_array[i]->outputDataTypeGiven)
      {//                       |     no
        xQueueSend(pPrintQueue, "    yes\n", 0);
      }
      else
      {
        xQueueSend(pPrintQueue, "     no\n", 0);
      }
	}
  }
}

