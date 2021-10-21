/**
 * @file operation.h
 * @brief Contain every data operation of each sensor implement in the EDUCAT project
 * @author YNCREA HDF / ISEN Lille
 */
#ifndef DATA_OPERATIONS_H_
#define DATA_OPERATIONS_H_

#include "structures.h"


int imu_data_to_ascii(imu_data_t * imu, char * dest, size_t * size);

int imu_data_to_bin(imu_data_t * imu, char * dest, size_t * size);

int imu_data_to_bin_no_timestamp(imu_data_t * imu, char * dest, size_t * size);

int pwc_data_to_bin(pwc_data_t * pwc, char * dest, size_t * size);
/* TODO: for all the functions, also try the data pointer */
int pwc_data_to_bin_no_timestamp(pwc_data_t * pwc, char * dest, size_t * size);


int pwc_data_to_ascii(pwc_data_t * pwc, char * dest, size_t * size);

int gps_data_to_bin_no_timestamp(gps_data_t * gps, char * dest, size_t * size);

/* The following function parses in binary data this way:
 * pwcType|
 * TODO next: implement the distance sensor node parse
 */
int all_data_to_binary(data_container_t * container, unsigned char * dest, size_t * size);
int all_data_to_ascii(data_container_t * container,  unsigned char * dest, size_t * size);

int imu6_data_to_int16_t_scaled(imu_data_t imu, unsigned int scale, unsigned char *dest);
int imu9r_data_to_int16_t_scaled(imu_data_t imu, unsigned int scale, unsigned char *dest);

int pwc_data_to_bin_no_mode_no_timestamp(pwc_data_t * pwc, char * dest, size_t * size);


/* Function to parse for data logging */
/* imu */
int imu_data_log(imu_data_t * imu, char * dest, size_t * size);
/* gps */
/* The part following is up to date with our protocol's specifications (see https://educat2seas.eu/data/api/setup/parameter/33/info)*/
int datatypeToRaw_joystick_dx2(pwc_data_t * pData, unsigned char *dest);
int datatypeToRaw_joystick_pg(pwc_data_t * pData, unsigned char *dest);
int datatypeToRaw_imu_9axis_rot_vec(imu_data_t * pImu, unsigned char *dest);
int datatypeToRaw_imu_6axis(imu_data_t * pImu, unsigned int scale, unsigned char *dest);
int datatypeToRaw_imu_quat(imu_100Hz_data_t * pImu, unsigned char *dest);
int datatypeToRaw_imu_quat_gyro_acc(imu_100Hz_data_t * pImu, unsigned char *dest);
int datatypeToRaw_imu_quat_gyro_acc_100Hz(imu_100Hz_data_t * pImu, unsigned char *dest);

int datatypeToRaw_usbad_instrument(usbad_data_t *pData, unsigned char *dest);
int datatypeToRaw_usbad_sensor_instrument(usbad_data_t *pData, unsigned char *dest);

int datatypeToRaw_canDistanceNodeD1(can_distance_node_d1 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD2(can_distance_node_d2 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD3(can_distance_node_d3 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD4(can_distance_node_d4 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD5(can_distance_node_d5 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD6(can_distance_node_d6 * pData, unsigned char *dest);

int datatypeToRaw_canDistanceNodeD7(can_distance_node_d6 * pData, unsigned char *dest);
int datatypeToRaw_canDistanceNodeD8(can_distance_node_d5 * pData, unsigned char * pDest);



int datatypeToRaw_gpsMinData(gps_data_t * pData, unsigned char *dest);


/* values defined below are use to avoid the overflows using snprintf */
#define datatypeToAscii_joystick_dx2_MAX_OUTPUT 64
#define datatypeToAscii_joystick_pg_MAX_OUTPUT  64
#define datatypeToAscii_joystick_linx_MAX_OUTPUT  64

#define datatypeToAscii_imu_9axis_MAX_OUTPUT    96
#define datatypeToAscii_imu_6axis_MAX_OUTPUT    64

#define datatypeToAscii_can_cd1_MAX_OUTPUT 32
#define datatypeToAscii_can_cd2_MAX_OUTPUT 32
#define datatypeToAscii_can_cd3_MAX_OUTPUT 64
#define datatypeToAscii_can_cd4_MAX_OUTPUT 64
#define datatypeToAscii_can_cd5_MAX_OUTPUT 64
#define datatypeToAscii_can_cd6_MAX_OUTPUT 64

#define datatypeToAscii_gps_minData_MAX_OUTPUT 64


/* The part following is up to date with our protocol's specifications (see https://educat2seas.eu/data/api/setup/parameter/33/info)*/
int datatypeToAscii_joystick_dx2(unsigned int instrumentType, pwc_data_t * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_joystick_pg(unsigned int instrumentType, pwc_data_t * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_imu_9axis_rot_vec(unsigned int instrumentType, imu_data_t * pImu, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_imu_6axis(unsigned int instrumentType, imu_data_t * pImu, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_canDistanceNodeD1(unsigned int instrumentType, can_distance_node_d1 * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_canDistanceNodeD2(unsigned int instrumentType, can_distance_node_d2 * pData, unsigned char *dest, unsigned int lineReturn);


int datatypeToAscii_canDistanceNodeD3(unsigned int instrumentType, can_distance_node_d3 * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_canDistanceNodeD4(unsigned int instrumentType, can_distance_node_d4 * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_canDistanceNodeD5(unsigned int instrumentType, can_distance_node_d5 * pData, unsigned char *dest, unsigned int lineReturn);
int datatypeToAscii_canDistanceNodeD6(unsigned int instrumentType, can_distance_node_d6 * pData, unsigned char *dest, unsigned int lineReturn);

int datatypeToAscii_gpsMinData(unsigned int instrumentType, gps_data_t * pData, unsigned char *dest, unsigned int lineReturn);


/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD1(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD2(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD3(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD4(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD5(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD6(unsigned char *source, unsigned int size, void * pData);

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD7(unsigned char *source, unsigned int size, void * pData);
/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD8(unsigned char * pSource, unsigned int size, void * pData);

int datatypeToRaw_joystick_linx(pwc_data_t * pData, unsigned char *dest);

int datatypeToRaw_RTC(uint64_t * pData, unsigned char *dest);

#endif /* DATA_OPERATIONS_H_ */
