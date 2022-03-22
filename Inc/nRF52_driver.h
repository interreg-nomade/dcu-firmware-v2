/*
 * nRF52_driver.h
 *
 *  Created on: 1 Dec 2021
 *      Author: sarah
 */

#ifndef NRF52_DRIVER_H_
#define NRF52_DRIVER_H_


#include "stdint.h"

//  _______________________________________________________________________________
// +------------+------------+---------------------+-----------+-----------+---------+--------+
// | START_BYTE | packet_len | command (DATA_BYTE) | sensor_nr | data_type | data    | CS     |
// +------------+------------+---------------------+-----------+-----------+---------+--------+
// |   1 byte   |   1 byte   |       1 byte        |  1 byte   |  1 byte   | k bytes | 1 byte |
// +------------+------------+---------------------+-----------+-----------+---------+--------+
//      0x73
//

#define BLE_GAP_ADDR_LEN                (6)
#define START_BYTE                      0x73 // s
#define OVERHEAD_BYTES                  6
#define PACKET_DATA_PLACEHOLDER         5
#define USR_INTERNAL_COMM_MAX_LEN       128
#define CONFIG_PACKET_DATA_OFFSET       3
#define CS_LEN                          1
#define MAX_CONNECTED_DEVICES			8

typedef enum
{
    DATA = 1,
    CONFIG
} command_byte_t;

typedef enum
{
    QUATERNIONS = 1,
    EULER,
    RAW
} data_type_byte_t;

typedef enum
{
    COMM_CMD_MEAS_RAW = 1,
    COMM_CMD_MEAS_QUAT6,
    COMM_CMD_MEAS_QUAT9,
    COMM_CMD_MEAS_WOM
} command_type_meas_byte_t;

typedef enum
{
    COMM_CMD_START_SYNC = 1,
    COMM_CMD_STOP_SYNC
} command_type_sync_byte_t;

typedef enum
{
    COMM_CMD_SET_CONN_DEV_LIST = 1,
    COMM_CMD_REQ_CONN_DEV_LIST,
    COMM_CMD_START,
    COMM_CMD_STOP,
    COMM_CMD_MEAS,
    COMM_CMD_SYNC,
    COMM_CMD_FREQUENCY,
    COMM_CMD_CALIBRATE,
    COMM_CMD_RESET,
    COMM_CMD_REQ_BATTERY_LEVEL,
	COMM_CMD_OK,
	COMM_CMD_UNKNOWN,
	COMM_CMD_SENSOR_CONNECT_STATUS
} command_type_byte_t;

typedef enum
{
    COMM_CMD_CALIBRATION_START = 1,
    COMM_CMD_CALIBRATION_DONE,
    COMM_CMD_CALIBRATION_GYRO_DONE,
    COMM_CMD_CALIBRATION_ACCEL_DONE,
    COMM_CMD_CALIBRATION_MAG_DONE,
} command_type_calibration_byte_t;

typedef struct
{
    uint8_t addr[BLE_GAP_ADDR_LEN]; /**< 48-bit address, LSB format. */
} dcu_conn_dev_t;

// IC = internal communication
typedef enum
{
    IC_EVT_QUAT,
    IC_EVT_RAW,
	IC_EVT_BATT,
    IC_EVT_CONN_DEV_LIST,
    IC_EVT_CALIBRATION_DONE,
} ic_evt_type_t;

typedef struct
{
    float w;
    float x;
    float y;
    float z;
}stm32_decoded_quat_t;

typedef struct
{
    float x;
    float y;
    float z;
} stm32_decoded_accel_t;

typedef struct
{
    float x;
    float y;
    float z;
} stm32_decoded_gyro_t;

typedef struct
{
    float x;
    float y;
    float z;
} stm32_decoded_compass_t;

typedef struct
{
    stm32_decoded_accel_t   accel;
    stm32_decoded_gyro_t    gyro;
    stm32_decoded_compass_t compass;
} stm32_decoded_raw_t;

typedef struct batt
{
    uint8_t level;
    float voltage;
} BATTERY;

typedef struct batt_array
{
    BATTERY batt[MAX_CONNECTED_DEVICES];
} BATTERY_ARRAY;

/**@brief Bluetooth Low Energy address. */
typedef struct
{
  uint8_t addr_id_peer : 1;       /**< Only valid for peer addresses.
                                       This bit is set by the SoftDevice to indicate whether the address has been resolved from
                                       a Resolvable Private Address (when the peer is using privacy).
                                       If set to 1, @ref addr and @ref addr_type refer to the identity address of the resolved address.

                                       This bit is ignored when a variable of type @ref ble_gap_addr_t is used as input to API functions. */
  uint8_t addr_type    : 7;       /**< See @ref BLE_GAP_ADDR_TYPES. */


//#define BLE_GAP_ADDR_TYPE_PUBLIC                        0x00 /**< Public address. */
//#define BLE_GAP_ADDR_TYPE_RANDOM_STATIC                 0x01 /**< Random Static address. */
//#define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE     0x02 /**< Private Resolvable address. */
//#define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE 0x03 /**< Private Non-Resolvable address. */


  uint8_t addr[BLE_GAP_ADDR_LEN]; /**< 48-bit address, LSB format. */
} ble_gap_addr_t;

// Match connection handles to unique IDs
typedef struct
{
  uint16_t conn_handle;
  ble_gap_addr_t addr;
} dcu_connected_devices_t;

typedef struct{
    ic_evt_type_t type;
    struct
    {
        stm32_decoded_quat_t quat;
        stm32_decoded_raw_t raw;
        BATTERY_ARRAY all_batt;
        dcu_connected_devices_t dev[MAX_CONNECTED_DEVICES];
        uint8_t sensor_nr;
    } data;
} ic_evt_t;

typedef  struct
{
    int32_t w;
    int32_t x;
    int32_t y;
    int32_t z;
} stm32_quat_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} stm32_accel_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} stm32_gyro_t;

typedef struct
{
    int16_t x;
    int16_t y;
    int16_t z;
} stm32_compass_t;

typedef struct
{
    stm32_accel_t   accel;
    stm32_gyro_t    gyro;
    stm32_compass_t compass;
} stm32_raw_t;

typedef struct tm stm32_datetime_t;
typedef uint64_t stm32_time_t;


// Initialize library: pass event handler + uart rx/tx functions
//void comm_init(const ic_init_t* p_init, const ic_uart_t* p_uart);

// Request a list of battery levels
void comm_req_batt_lvl();

// Decode a received packet via internal communication between nRF52 and STM32
void comm_decode_rx_packet(uint8_t* rx_data, uint32_t len);

// Request a list of connected devices with corresponding connection handles
void comm_req_conn_dev();

// Set a list of MAC addresses where the sensors should connect to
void comm_set_mac_addr(dcu_conn_dev_t addr[], uint32_t len);

// Enable or disable synchronization between sensors
void comm_set_sync(command_type_sync_byte_t sync);

// Set the required metrics to be send from the IMUs
void comm_set_data_type(command_type_meas_byte_t type);

// Set the measuring frequency of all IMUs
void comm_set_frequency(uint8_t freq);

// Start measurement
void comm_start_meas();

// Start measurement including time stamp
void comm_start_meas_w_time(stm32_datetime_t *dateTime);

// Stop measurement
void comm_stop_meas();

// Send calibration command to all connected devices
void comm_calibrate();

#endif /* NRF52_DRIVER_H_ */
