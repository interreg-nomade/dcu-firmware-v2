/**
 * @file submsgs.h
 * @brief Contains sub message related functions and definitions
 */


#ifndef DATA_SUBMSGS_H_
#define DATA_SUBMSGS_H_

#include "data/structures.h"

#define SUB_MSG_ID_TIMESTAMP		0x0F

#define SUB_MSG_ID_DX2_OUTPUT		0xA1
#define SUB_MSG_ID_PG_OUTPUT		0xA2
#define SUB_MSG_ID_LINX_OUTPUT		0xA3


#define SUB_MSG_ID_IMU_9R_OUTPUT	0xB1
#define SUB_MSG_ID_IMU_6_OUTPUT		0xB2

#define SUB_MSG_ID_GPS_MIN_OUTPUT	0xC1
#define SUB_MSG_ID_GPS_STATUS		0xC2
#define SUB_MSG_ID_GPS_DATA_STATUS	0xC3

#define SUB_MSG_ID_DIST_NODES_LIST	0xD1

#define SUB_MSG_ACCEL_SCALE			100.0
#define SUB_MSG_GYRO_SCALE			100.0
#define SUB_MSG_MAG_SCALE			100.0
#define SUB_MSG_ROTV_SCALE			100.0



/**
 * @brief Function to parse a timestamp value as a submessage
 * @param timestamp input the timestamp to parse
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_Timestamp(unsigned int timestamp, unsigned char * dest);



/**
 * @brief Function to parse output data from a DX2 system as a submessage
 * @param data DX2 PWC output data contained in a pwc_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_Dx2Output(pwc_data_t data, unsigned char * dest);



/**
 * @brief Function to parse output data from a PG R-Net system as a submessage
 * @param data PG PWC output data contained in a pwc_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_PgOutput(pwc_data_t data, unsigned char * dest);



/**
 * @brief Function to parse output data from a Dynamic Control's LinX system as a submessage
 * @param data LinX PWC data contained in a pwc_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_LinxOutput(pwc_data_t data, unsigned char * dest);



/**
 * @brief Function to parse output data from any PWC's system reduced in a pwc_data_t structure
 * as a submessage / Note: the type is checked inside the function.
 * @param data LinX PWC data contained in a pwc_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_PwcGenericOutput(pwc_data_t data, unsigned char * dest);



/**
 * @brief Function to parse 9 axis + rotation vectors output data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_9rOutput(imu_data_t * data, unsigned char * dest);



/**
 * @brief Function to parse 6 axis output data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_6Output(imu_data_t * data, unsigned char * dest);



/**
 * @brief Function to parse GPS data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsMinOutput(gps_data_t data, unsigned char * dest);



/**
 * @brief Function to parse a gps status as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsStatus(gps_data_t data, unsigned char * dest);


/**
 * @brief Function to parse a gps status + data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsStatusData(gps_data_t data, unsigned char * dest);

/**
 * @brief Function to parse a distance node list data + data as a submessage
 * @param data distance data contained in a distance_node_list_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_distanceNodeList(distance_node_list_t data, unsigned char * dest);


#endif /* DATA_SUBMSGS_H_ */
