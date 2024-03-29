/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: usb_com.c
 *      Created: 2020-02-27
 *       Author: Jarne Van Mulders
 *      Version: V1.0
 *
 *  Description: Firmware IMU sensor module for the NOMADe project
 *
 *  Interreg France-Wallonie-Vlaanderen NOMADe
 *
 */
#include "usb_com.h"
#include "usart.h"
#include <string.h>
#include "../app/app_init.h" // to declare QueueHandle_t

#define PRINTF_USB_COM 1


// ================================================================
// ===                    		Variables                      		===
// ================================================================

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];
extern uint8_t state;

extern char string[];
extern QueueHandle_t pPrintQueue;

uint32_t last_sync_started_time = 0;

// ================================================================
// ===                     		Functions                     		===
// ================================================================

void USB_COM_show_menu(void){
	  xQueueSend(pPrintQueue, "\n\n########################################################################\n", 0);
	  xQueueSend(pPrintQueue, "					Command list\n", 0);
	  xQueueSend(pPrintQueue, "________________________________________________________________________\n", 0);
	  xQueueSend(pPrintQueue, "Command \"0\": Print MENU\n", 0);
	  xQueueSend(pPrintQueue, "________________________________________________________________________\n", 0);
	  xQueueSend(pPrintQueue, "			General functions\n", 0);
	  xQueueSend(pPrintQueue, "- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - \n", 0);
	  xQueueSend(pPrintQueue, "Command \"1\": Send connected device list to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"2\": Request connected device list to nRF52.\n", 0);
	  xQueueSend(pPrintQueue, "Command \"3\": Request to start measurement to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"4\": Request to stop measurement to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"5\": Send output data type for all modules to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"6\": Request to start synchronization to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"7\": Request to stop synchronization to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"8\": Request calibration of connected modules to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"9\": Reset nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"a\": Request battery level of connected modules to nRF52\n", 0);
	  xQueueSend(pPrintQueue, "Command \"f\": Change sampling frequency\n", 0);
	  xQueueSend(pPrintQueue, "Command \"m\": Show overview status of module library\n", 0);
	  xQueueSend(pPrintQueue, "________________________________________________________________________\n", 0);
	  xQueueSend(pPrintQueue, "########################################################################\n\n", 0);
}

void USB_COM_print(const char* string)
{
  xQueueSend(pPrintQueue, string, 0);
}

void USB_COM_print_ln(const char* string)
{
  strcat(string, "\n");
  xQueueSend(pPrintQueue, string, 0);
}

void USB_COM_print_value_ln(const char* str, uint32_t value)
{
  char char_array [100];
  sprintf(char_array, str, value);
  UART_COM_write(&huart7, (uint8_t *)char_array, strlen(char_array));
}

void USB_COM_print_info(const char* str, const char* str2)
{
	UART_COM_print(&huart7, str);
	UART_COM_print_ln(&huart7, str2);
}

void USB_COM_print_buffer_hex(uint8_t *buffer, uint8_t len)
{
  len--;
  sprintf(string, "");
  char DUString[3];
  for (uint8_t i = len; i >= 0; i--)
  {
    if (strlen(string) < 147)
  	{
	  sprintf(DUString, "%02X",buffer[i]);
	  strcat(string, DUString);
  	}
  }
  strcat(string,"\n");
  xQueueSend(pPrintQueue, string, 0);
}

void USB_COM_print_buffer_mac_address(const char* str, uint8_t *mac_address)
{
	USB_COM_print(str);
	USB_COM_print("MAC Address: ");
	USB_COM_print_buffer_hex(mac_address, 6);
}

void USB_COM_change_frequency_menu(void){
	USB_COM_print_ln("________________________________________________________________________");
	USB_COM_print_ln("			MENU -- Sample frequency options");
	USB_COM_print_ln("- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -");
	USB_COM_print_ln("Command \"1\": Sampling frequency 10 Hz");
	USB_COM_print_ln("Command \"2\": Sampling frequency 20 Hz");
	USB_COM_print_ln("Command \"3\": Sampling frequency 25 Hz");
	USB_COM_print_ln("Command \"4\": Sampling frequency 50 Hz");
	USB_COM_print_ln("Command \"5\": Sampling frequency 100 Hz");
	USB_COM_print_ln("________________________________________________________________________");
}




