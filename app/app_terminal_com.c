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
#include "interface_sd.h"
#include "../Inc/imu_com.h"
#include "app_rtc.h"

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

extern imu_module *imu_array [];

extern Status_SD SD_Status;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint8_t previous_command = 0;
imu_module *imu = NULL;
uint8_t previous_connected_modules [6];
uint8_t sync_enable;

char command;

void terminalComManagerThread(const void *params);


// ================================================================
// ===                    Static functions                      ===
// ================================================================

static uint8_t is_measuring(void);
static void send_to_all_sensors(void (*function)(imu_module*), imu_module *imu_array);
static void send_to_all_sensors_with_param(void (*function)(imu_module*, uint8_t), imu_module *imu_array, uint8_t df);
static uint8_t convert_ASCII_to_HEX(uint8_t msn, uint8_t lsn);
static void read_mac_address(imu_module *imu);


static int com_Build(void);


void com_init_rx_task(void)
{
    osDelay(100);
    /* Init the decoder */
    terminal_com_init();

#if PRINTF_TERMINAL_COM
    sprintf(string, "terminal_com_init_rx_task done. \n");
    HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
#endif

    memset(&ComRxMsg, 0, sizeof(com_msg_t));

    osThreadDef(terminalComManager, terminalComManagerThread, osPriorityNormal, 0, 1024);
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
#if PRINTF_TERMINAL_COM
			sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Data received via Terminal: 0x%2X\n",(unsigned int) HAL_GetTick(),command);
    	      xQueueSend(pPrintQueue, string, 0);
#endif
			if(command == 0x0A) return;
			if(previous_command != 0)
			{
				switch(command)
				{
					case 0x31: // "1"
					{
						imu = &imu_1;
						if (0x66 == previous_command)
						{
							send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_10HZ);
						}
					} break;
					case 0x32: // "2"
					{
						imu = &imu_2;
						if (0x66 == previous_command)
						{
							send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_20HZ);
						}
					} break;
					case 0x33: // "3"
					{
						imu = &imu_3;
						if(0x66 == previous_command)
						{
							send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_25HZ);
						}
					} break;
					case 0x34: // "4"
					{
						imu = &imu_4;
						if(0x66 == previous_command)
						{
							send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_50HZ);
						}
					} break;
					case 0x35: // "5"
					{
						imu = &imu_5;
						if(0x66 == previous_command)
						{
							send_to_all_sensors_with_param(IMU_change_sampling_frequency, *imu_array, SAMPLING_FREQ_100HZ);
						}
					} break;
					case 0x36: // "6"
					{
						imu = &imu_6;
					} break;
					default:
						imu = NULL;
						sprintf(string, "Please enter a number between 1 and 6!\n");
			            xQueueSend(pPrintQueue, string, 0);
				}
				command = previous_command;
				previous_command = 0;
			}
			switch(command)
			{
				case 0x30:  // "0": Print MENU
				{
					USB_COM_show_menu();
				}
				break;
				case 0x31: // "1": Connect
				{
					if(imu != NULL)
					{
						sprintf(string, "%u [app_terminal_com] [terminalComManagerThread] Connect IMU 0x%1X with MAC Address 0x%2X.\n",(unsigned int) HAL_GetTick(),imu->number,*imu->mac_address);
			      	    xQueueSend(pPrintQueue, string, 0);
						IMU_connect(imu);
						imu = NULL;
					}
					else
					{
						previous_command = 0x31;
						sprintf(string, "IMU number: \n");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x32:  // "2": Disconnect
				{
					if(imu != NULL)
					{
						IMU_disconnect(imu);
						imu = NULL;
					}
					else
					{
						previous_command = 0x32;
						sprintf(string, "IMU number: ");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x33: // "3" Go to sleep
				{
					if(imu != NULL)
					{
						IMU_go_to_sleep(imu);
						imu = NULL;
					}
					else
					{
						previous_command = 0x33;
						sprintf(string, "IMU number: ");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
//					for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
//					{
//						if(imu_array[i]->connected)	IMU_go_to_sleep(imu_array[i]);
//					}
				}
				break;
				case 0x34:  //"4": Get the battery voltage
				{
//					for(uint8_t i = 0; i < 6; i++)
//					{
//						if(imu_array[i]->connected)	IMU_get_battery_voltage(imu_array[i]);
//					}
					IMU_get_battery_voltage(&imu_1);
				}
				break;
				case 0x35:  //"5": Get the system tick
				{
					compare_RTCunixtime_Epoch();
					for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
					{
						if(imu_array[i]->connected)	IMU_get_systicks(imu_array[i]);
					}
				}
				break;
				case 0x36:  //"6": Print MAC addresses to which the BLE slots should connect
				{
					for (uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
					{
						USB_COM_print_buffer_mac_address(imu_array[i]->name, imu_array[i]->mac_address);
					}
				}
				case 0x37:  //"7": Get synchronization time
				{
					for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
					{
						if(imu_array[i]->connected)	IMU_get_sync_time(imu_array[i]);
					}
				}
				break;
				case 0x38:  //"8": IMU adapt synchronization
				{
					IMU_synchronisation_adaptation(*imu_array);
				}
				break;
				case 0x39:  //"9": Change MAC address of a BLE slot
				{
					if(imu != NULL)
					{
						read_mac_address(imu);
						imu = NULL;
					}
					else
					{
						previous_command = 0x39;
						sprintf(string, "Slot number: ");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x73: //"s": Start synchronization
				{
					for(uint8_t i = 0; i < 6; i++)
					{
						if(imu_array[i]->connected)
						{
							IMU_start_synchronisation(imu_array[i]);
							previous_connected_modules [i] = 1;
						}
					}
					IMU_send_adv_msg_wrong(&imu_1);
					osDelay(250);
					IMU_send_adv_msg(&imu_1);
//					last_sync_started_time = HAL_GetTick();
//					for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
//					{
//						if(imu_array[i]->connected)
//						{
//							IMU_start_synchronisation(imu_array[i]);
//							previous_connected_modules [i] = 1;
//						}
//					}
//					IMU_sync_reset();
//					HAL_Delay(10);
//					sync_enable = 1;
//					IMU_send_adv_msg_wrong(&imu_1);
				}
				break;
				case 0x63:  // "c": Calibration
				{
//					for(uint8_t i = 0; i < 6; i++)
//					{
//						if(imu_array[i]->connected)	IMU_start_calibration(imu_array[i]);
//					}
					IMU_start_calibration(&imu_1);
//					send_to_all_sensors(IMU_start_calibration, *imu_array);
				}
				break;
				case 0x64:  // "d": get time & date
				{
					app_rtc_print_RTCdateTime();
				}
				break;
				case 0x6E: // "n": SD card function - Create new file
				{
					if (!is_measuring())
					{
						SD_CARD_COM_create_new_file();
					}
					else
					{
						sprintf(string, "Measurement ongoing. End measurement first before generating a new file.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x6D: // "m": SD card function - Mount SD card
				{
					SD_CARD_COM_mount();
				}
				break;
				case 0x75: // "u": SD card function - Unmount SD card
				{
					if (!is_measuring())
					{
						SD_CARD_COM_unmount();
					}
					else
					{
						sprintf(string, "Measurement ongoing. End measurement first before un-mounting the SD Card.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x66: // "f": Change sampling frequency
				{
					if(imu != NULL)
					{
						imu = NULL;
					}
					else
					{
						previous_command = 0x66;
						USB_COM_change_frequency_menu();
						sprintf(string, "Give number: ");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x72: // "r": Start the measurement with synchronization
				{
					if(SD_Status.status == SD_MOUNTED)
					{
						sprintf(string, "Start measurement.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
						SD_CARD_COM_open_file();
						IMU_start_measurements(&imu_1);
					}
					else
					{
						sprintf(string, "Measurements not started, SD card not available.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x74: // "t": Start the measurement without synchronization
				{
					if(SD_Status.status == SD_MOUNTED)
					{
						sprintf(string, "Start measurement.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
						SD_CARD_COM_open_file();
						IMU_start_measurements_without_sync(&imu_1);
					}
					else
					{
						sprintf(string, "No available SD card found.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
					}
				}
				break;
				case 0x65:  // "e": End the measurement
				{
					if(is_measuring())
					{
//						sprintf(string, "End the measurement.");
//						HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//						SD_CARD_COM_close_file();
						IMU_stop_measurements(&imu_1);
					}
					else
					{
						sprintf(string, "Start a measurement first.\n");
			      	    xQueueSend(pPrintQueue, string, 0);
						IMU_stop_measurements(&imu_1);

					}

				}
				break;
				case 0x67: // "g": Change data format - Only Quaternion data
				{
					send_to_all_sensors_with_param(IMU_change_dataformat, *imu_array, DATA_FORMAT_1);
				}
				break;
				case 0x68: // "h": Change data format - Only Gyroscope data
				{
					send_to_all_sensors_with_param(IMU_change_dataformat, *imu_array, DATA_FORMAT_2);
				}
				break;
				case 0x6A: // "j": Change data format - Only Accelerometer data
				{
					send_to_all_sensors_with_param(IMU_change_dataformat, *imu_array, DATA_FORMAT_3);
				}
				break;
				case 0x6B: // "k": Change data format - Gyroscope + Accelerometer data
				{
					send_to_all_sensors_with_param(IMU_change_dataformat, *imu_array, DATA_FORMAT_4);
				}
				break;
				case 0x6C: // "l": Change data format - Quaternion + Gyroscope + Accelerometer data
				{
					send_to_all_sensors_with_param(IMU_change_dataformat, *imu_array, DATA_FORMAT_5);
				}
				break;
				case 0x61: // "a": Get the IMU module status
				{
					send_to_all_sensors(IMU_get_status, *imu_array);
				}
				break;
				case 0x7A: // "z": Get the IMU module software version
				{
					send_to_all_sensors(IMU_get_software_version, *imu_array);
				}
				break;
				case 0x0A: //"<ENTER>"
				{
					// It's just an enter (captered)
				}
				break;
				default:
					sprintf(string, "Undefined command.\n");
		      	    xQueueSend(pPrintQueue, string, 0);
			}
		}
		osDelay(100); // every 100 HAL Ticks
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
#if PRINTF_TERMINAL_COM
		sprintf(string, "%u [app_terminal_com] [com_Build] Terminal ring buffer not empty.\n",(unsigned int) HAL_GetTick());
  	    xQueueSend(pPrintQueue, string, 0);
#endif
		if (!ring_buffer_is_empty(&comRingbufRx))
	    {
	        ring_buffer_dequeue(&comRingbufRx, &rxdByte);
#if PRINTF_TERMINAL_COM
		sprintf(string, "%u [app_terminal_com] [com_Build] Received byte: 0x%2X %c\n",(unsigned int) HAL_GetTick(),rxdByte,rxdByte);
  	    xQueueSend(pPrintQueue, string, 0);
#endif
	    }
	    command = rxdByte;
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

uint8_t is_measuring(void){
	uint8_t rt_val = 0;
	for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++){
		if(imu_array[i]->measuring) rt_val = 1;
	}
	return rt_val;
}

void send_to_all_sensors(void (*function)(imu_module*), imu_module *imu_array){
	for(uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++){
		if(imu_array[i].connected) function(&imu_array[i]);
	}
}

void send_to_all_sensors_with_param(void (*function)(imu_module*, uint8_t), imu_module *imu_array, uint8_t df)
{
  for (uint8_t i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
  {
    if (imu_array[i].connected)
    {
      function(&imu_array[i], df);
#if PRINTF_TERMINAL_COM
	  sprintf(string, "%u [app_terminal_com] [send_to_all_sensors_with_param] Adapted for slot number 0x%X.\n",(unsigned int) HAL_GetTick(),i);
	  xQueueSend(pPrintQueue, string, 0);
#endif
	}
    else
    {
#if PRINTF_TERMINAL_COM
	  sprintf(string, "%u [app_terminal_com] [send_to_all_sensors_with_param] Slot number 0x%X not connected.\n",(unsigned int) HAL_GetTick(),i);
	  xQueueSend(pPrintQueue, string, 0);
#endif
    }
  }
}

void read_mac_address(imu_module *imu){
	HAL_Delay(1);

	uint8_t new_mac_adress [6];

	for(uint8_t i = 0; i < 6; i++){
		uint8_t a, b;
		if(UART_IsDataAvailable(&huart5))	a = UART_COM_read(&huart5);
		if(UART_IsDataAvailable(&huart5))	b = UART_COM_read(&huart5);
		new_mac_adress [i] = convert_ASCII_to_HEX(a, b);
	}

	IMU_adjust_mac_address(imu, new_mac_adress);
	USB_COM_print_buffer_mac_address(imu->name, imu->mac_address);

}

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


