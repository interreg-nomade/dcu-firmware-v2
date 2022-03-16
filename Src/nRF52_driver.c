/*
 * nRF52_driver.c
 *
 *  Created on: 1 Dec 2021
 *      Author: sarah
 */

#include <string.h>
#include "../app/app_init.h" // to declare QueueHandle_t
#include "stm32h7xx_hal.h"
#include "usart.h"
#include "nRF52_driver.h"
#include "../app/data/structures.h"

#define PRINTF_nRF52_DRIVER 1

extern UART_HandleTypeDef huart4;

extern char string[];
extern QueueHandle_t pPrintQueue;

// Static function prototypes
//static void decode_quat(stm32_quat_t* in, stm32_decoded_quat_t* out);
//static void decode_raw(stm32_raw_t* in, stm32_decoded_raw_t* out);
static uint8_t calculate_cs(uint8_t * data, uint32_t * len);
static void check_buffer_overflow(uint32_t* len);



static void send_data(command_type_byte_t type, uint8_t* data, uint32_t len)
{
    uint8_t tx_buffer[USR_INTERNAL_COMM_MAX_LEN]; //64 bytes long is more than enough for a data packet
    uint32_t tx_buffer_len;

    command_byte_t command_byte;

    // Fill configuration bytes
    tx_buffer[0] = START_BYTE;

    tx_buffer_len = 0;
    // Length of frame
    tx_buffer_len += (OVERHEAD_BYTES-1);

    // Tell the receiver its data we're sending
    command_byte = CONFIG;

    tx_buffer[2] = command_byte;
    tx_buffer[3] = type;

    // Copy data to packet
    memcpy((tx_buffer + PACKET_DATA_PLACEHOLDER-1), data, len);

    // Set data len
    tx_buffer_len += len;
    tx_buffer[1] = (uint8_t) tx_buffer_len;

    // Checksum
    uint8_t cs = calculate_cs(tx_buffer, &tx_buffer_len);
    tx_buffer[PACKET_DATA_PLACEHOLDER-1 + len] = cs;

    // check for buffer overflows
    check_buffer_overflow(&tx_buffer_len);

    // Send over UART to STM32
    //m_uart->tx(data_out, data_len);

#if PRINTF_nRF52_DRIVER
  sprintf(string, "%u [nRF52_driver] [send data] transmit buffer: ",(unsigned int) HAL_GetTick());
  xQueueSend(pPrintQueue, string, 0);
  sprintf(string, "");
  char DUString[3];
  for (int i = 0; i < (tx_buffer_len); i++)
  {
    if (strlen(string) < 147)
  	{
      sprintf(DUString, "%02X",tx_buffer[i]);
  	  strcat(string, DUString);
    }
  }
  strcat(string,"\n");
  xQueueSend(pPrintQueue, string, 0);
#endif
    HAL_UART_Transmit(&huart4, (uint8_t *)tx_buffer, tx_buffer_len, 25);
}

// WORKING
void comm_req_batt_lvl()
{
	send_data(COMM_CMD_REQ_BATTERY_LEVEL, NULL, 0);
}

// WORKING
void comm_req_conn_dev()
{
    send_data(COMM_CMD_REQ_CONN_DEV_LIST, NULL, 0);
}

// WORKING
void comm_set_mac_addr(dcu_conn_dev_t addr[], uint32_t len) // Needs to be an 8 address long array
{
    send_data(COMM_CMD_SET_CONN_DEV_LIST, (uint8_t*)addr, len);
}

// WORKING
void comm_set_sync(command_type_sync_byte_t sync)
{
    send_data(COMM_CMD_SYNC, &sync, sizeof(sync));
}

// WORKING
void comm_set_data_type(command_type_meas_byte_t type)
{
    send_data(COMM_CMD_MEAS, &type, sizeof(type));
}

// WORKING
void comm_set_frequency(uint8_t freq)
{
    send_data(COMM_CMD_FREQUENCY, &freq, sizeof(freq));
}

// WORKING
void comm_start_meas()
{
    send_data(COMM_CMD_START, NULL, 0);
}

void comm_start_meas_w_time(stm32_datetime_t *dateTime)
{

	time_t usr_time = mktime(dateTime);
	usr_time *= 1000; // Convert to ms

	stm32_time_t time = (stm32_time_t) usr_time;

	send_data(COMM_CMD_START, &time, sizeof(time));
}

// WORKING
void comm_stop_meas()
{
    send_data(COMM_CMD_STOP, NULL, 0);
}


void comm_calibrate()
{
	send_data(COMM_CMD_CALIBRATE, NULL, 0);
}

// WORKING
static uint8_t calculate_cs(uint8_t * data, uint32_t * len)
{
    // Init to zero
    uint8_t cs = 0x00;

    for (uint32_t i=0; i<(*len-1); i++)
    {
        // XOR
        cs ^= data[i];
    }

    return cs;
}

// Error checking helper functions
static void check_buffer_overflow(uint32_t* len)
{
    if(*len >= USR_INTERNAL_COMM_MAX_LEN)
    {
        //error_handler(STM32_ERROR_BUFFER_OVERFLOW);
    }
}

//// UART WORKING
//// EVT HANDLER WORKING
//void comm_init(const ic_init_t* p_init, const ic_uart_t* p_uart)
//{
//    m_callback = p_init->evt_handler;
//    m_uart = p_uart;
//}

