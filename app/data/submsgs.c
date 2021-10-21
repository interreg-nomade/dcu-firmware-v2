/*
 * submsgs.c
 *
 *  Created on: Nov 22, 2018
 *      Author: aclem
 */

#include "submsgs.h"

#include "data/operations.h"
#include "data_op/op.h"

//#include "stm32h7xx_hal_conf.h" /* For debug only / assert param use */

/**
 * @brief Function to parse a timestamp value as a submessage
 * @param timestamp input the timestamp to parse
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_Timestamp(unsigned int timestamp, unsigned char * dest)
{
	//assert_param(!dest);

	*dest++ = (unsigned char) SUB_MSG_ID_TIMESTAMP;
	rightshifty32_unsigned_integer_msb(timestamp, dest);

	return 5;
}



/**
 * @brief Function to parse output data from a DX2 system as a submessage
 * @param data DX2 PWC output data contained in a pwc_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_Dx2Output(pwc_data_t data, unsigned char * dest)
{
	//assert_param(!dest);
	/* Write the ID */
	*dest++ = SUB_MSG_ID_DX2_OUTPUT;
	/* Write the content of the submessage */
	*dest++ =  data.turn;
	*dest++ =  data.speed;
	*dest   = (data.isOnline << 7) | data.profile;

	return 4;
}



/**
 * @brief Function to parse output data from a PG R-Net system as a submessage
 * @param data PG PWC output data contained in a pwc_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_PgOutput(pwc_data_t data, unsigned char * dest)
{
	//assert_param(!dest);
	/* Write the ID */
	*dest++ = SUB_MSG_ID_PG_OUTPUT;
	/* Write the content of the submessage */
	*dest++ =  data.turn;
	*dest++ =  data.speed;
	*dest++ = (data.isOnline << 7) | data.profile;
	*dest   =  data.mode;

	return 5;
}



/**
 * @brief Function to parse output data from a Dynamic Control's LinX system as a submessage
 * @param data LinX PWC data contained in a pwc_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_LinxOutput(pwc_data_t data, unsigned char * dest)
{
	//assert_param(!dest);
	/* TODO /// Not implemented yet */
	return 1;
}



/**
 * @brief Function to parse output data from any PWC's system reduced in a pwc_data_t structure
 * as a submessage / Note: the type is checked inside the function.
 * @param data PWC data contained in a pwc_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_PwcGenericOutput(pwc_data_t data, unsigned char * dest)
{
	//assert_param(!dest);

	switch(data.pwcType)
	{
		case PWC_TYPE_DX:
		{
			return (subMsgParser_Dx2Output(data, dest));
			break;
		}
		case PWC_TYPE_PG:
		{
			return (subMsgParser_PgOutput(data, dest));
			break;
		}
		case PWC_TYPE_LINX:
		{
			return (subMsgParser_LinxOutput(data, dest));
			break;
		}
		default:
		{
			return 0;
			/* Error */
			break;
		}
	}
	return 0;
}



/**
 * @brief Function to parse 9 axis + rotation vectors output data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_9rOutput(imu_data_t * data, unsigned char * dest)
{
	//assert_param(!dest);

	/* Apply the scales to the data */
#if 0
	data->accelerometer.x *= SUB_MSG_ACCEL_SCALE;
	data->accelerometer.y *= SUB_MSG_ACCEL_SCALE;
	data->accelerometer.z *= SUB_MSG_ACCEL_SCALE;
	data.gyroscope.x     *= SUB_MSG_GYRO_SCALE;
	data.gyroscope.y     *= SUB_MSG_GYRO_SCALE;
	data.gyroscope.z     *= SUB_MSG_GYRO_SCALE;
	data.magnetometer.x  *= SUB_MSG_MAG_SCALE;
	data.magnetometer.y  *= SUB_MSG_MAG_SCALE;
	data.magnetometer.z  *= SUB_MSG_MAG_SCALE;
	data.rotVectors.real *= SUB_MSG_ROTV_SCALE;
	data.rotVectors.i    *= SUB_MSG_ROTV_SCALE;
	data.rotVectors.j    *= SUB_MSG_ROTV_SCALE;
	data.rotVectors.k    *= SUB_MSG_ROTV_SCALE;
#endif
	short ax, ay, az = 0;
	short gx, gy, gz = 0;
	short mx, my, mz = 0;
	short real, i, j, k = 0;

	ax = (short) (data->accelerometer.x * SUB_MSG_ACCEL_SCALE);
	ay = (short) (data->accelerometer.y * SUB_MSG_ACCEL_SCALE);
	az = (short) (data->accelerometer.z * SUB_MSG_ACCEL_SCALE);

	gx = (short) (data->gyroscope.x * SUB_MSG_GYRO_SCALE);
	gy = (short) (data->gyroscope.y * SUB_MSG_GYRO_SCALE);
	gz = (short) (data->gyroscope.z * SUB_MSG_GYRO_SCALE);

	mx = (short) (data->magnetometer.x * SUB_MSG_MAG_SCALE);
	my = (short) (data->magnetometer.y * SUB_MSG_MAG_SCALE);
	mz = (short) (data->magnetometer.z * SUB_MSG_MAG_SCALE);

	real = (short) (data->rotVectors.real * SUB_MSG_ROTV_SCALE);
    i    = (short) (data->rotVectors.i * SUB_MSG_ROTV_SCALE);
	j    = (short) (data->rotVectors.j * SUB_MSG_ROTV_SCALE);
	k    = (short) (data->rotVectors.k * SUB_MSG_ROTV_SCALE);

	//TEST
	//printf("ax: %hi, ay: %hi, az: %hi\n", ax, ay, az);

	uintptr_t index = 0;

	*(dest) = SUB_MSG_ID_IMU_9R_OUTPUT;

	index = 1;
#if 0
	index += short_to_byte_array_msb((short) data.accelerometer.x, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.accelerometer.y, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.accelerometer.z, (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) data.gyroscope.x,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.gyroscope.y,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.gyroscope.z,     (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) data.magnetometer.x,  (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.magnetometer.y,  (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.magnetometer.z,  (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) data.rotVectors.real, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.rotVectors.i,    (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.rotVectors.j,    (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.rotVectors.k,    (char*) ( dest + index ));
#endif
	index += short_to_byte_array_msb((short) ax, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) ay, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) az, (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) gx,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) gy,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) gz,     (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) mx,  (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) my,  (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) mz,  (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) real, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) i,    (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) j,    (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) k,    (char*) ( dest + index ));

	return 27;
}



/**
 * @brief Function to parse 6 axis output data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_6Output(imu_data_t * data, unsigned char * dest)
{
	//assert_param(!dest);

	/* Previous implementation */
#if 0
	index = 1;

	index += short_to_byte_array_msb((short) data.accelerometer.x, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.accelerometer.y, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.accelerometer.z, (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) data.gyroscope.x,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.gyroscope.y,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) data.gyroscope.z,     (char*) ( dest + index ));
#else
	short ax, ay, az = 0;
	short gx, gy, gz = 0;
	short mx, my, mz = 0;
	short real, i, j, k = 0;

	ax = (short) (data->accelerometer.x * SUB_MSG_ACCEL_SCALE);
	ay = (short) (data->accelerometer.y * SUB_MSG_ACCEL_SCALE);
	az = (short) (data->accelerometer.z * SUB_MSG_ACCEL_SCALE);

	gx = (short) (data->gyroscope.x * SUB_MSG_GYRO_SCALE);
	gy = (short) (data->gyroscope.y * SUB_MSG_GYRO_SCALE);
	gz = (short) (data->gyroscope.z * SUB_MSG_GYRO_SCALE);


	//TEST
	//printf("ax: %hi, ay: %hi, az: %hi\n", ax, ay, az);

	uintptr_t index = 0;

	*(dest) = SUB_MSG_ID_IMU_6_OUTPUT;

	index = 1;


	index += short_to_byte_array_msb((short) ax, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) ay, (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) az, (char*) ( dest + index ));

	index += short_to_byte_array_msb((short) gx,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) gy,     (char*) ( dest + index ));
	index += short_to_byte_array_msb((short) gz,     (char*) ( dest + index ));

#endif
	return 13;
}



/**
 * @brief Function to parse GPS data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsMinOutput(gps_data_t data, unsigned char * dest)
{
	//assert_param(!dest);

	*dest++ = SUB_MSG_ID_GPS_MIN_OUTPUT;

	return 0;
}



/**
 * @brief Function to parse a gps status as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsStatus(gps_data_t data, unsigned char * dest)
{
	//assert_param(!dest);

	*dest++ = SUB_MSG_ID_GPS_STATUS;
	return 0;
	/* Not implemented yet ! */
	//TODO Code this function
}


/**
 * @brief Function to parse a gps status + data as a submessage
 * @param data Full IMU data contained in a imu_data_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_GpsStatusData(gps_data_t data, unsigned char * dest)
{
	//assert_param(!dest);

	*dest++ = SUB_MSG_ID_GPS_DATA_STATUS;
	return 0;
	/* Not implemented yet ! */
	//TODO Code this function
}

/**
 * @brief Function to parse a distance node list data + data as a submessage
 * @param data distance data contained in a distance_node_list_t structure
 * @param dest pointer to the destination buffer
 * @return the number of bytes written in the destination buffer in case of success ; 0 in case of failure
 */
int subMsgParser_distanceNodeList(distance_node_list_t data, unsigned char * dest)
{
	//assert_param(!dest);
	if (!dest){
		return 0;
	}
	unsigned int index = 0;

	*dest++ = SUB_MSG_ID_DIST_NODES_LIST;
	*dest++ = data.nDistNodes;

	index = 3;
	for (unsigned int i = 0; i<data.nDistNodes; i++)
	{

		*dest++ = data.distanceNodes[i].ID;
		unsigned short tmp = data.distanceNodes[i].distance;
		*dest++ = (tmp >> 8) & 0xff;
		*dest++ =  tmp		 & 0xff;
	}

	return (1+(3*data.nDistNodes));
}
