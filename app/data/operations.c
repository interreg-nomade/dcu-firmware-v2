/**
 * @file operation.c
 * @brief Source code on the data return by the sensor in the EDUCAT project
 * @author YNCREA HDF / ISEN Lille
 */

/* Standard libraries */
//#include <data/structures.h>
#include "structures.h"
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include "usart.h" // to be able to use print

#include "operations.h"
#include "app_init.h" // to declare QueueHandle_t

/* Custom libraries */
/* Util functions for the parsing */
#include "data_op/op.h"

extern char string[];
extern QueueHandle_t pPrintQueue;

#define PRINTF_OPERATIONS_DBG 1



/* In general: two types of operations:
 * 1. Structure to ASCII
 *
 * 		({struct_name}_to_ascii)
 *
 * 2. Structure to Binary (ex: 0xBBEEEEFF will be byte-split to 0xBB | 0xEE | 0xEE | 0xFF)
 *
 * 		({struct_name}_to_bin)
 */

int imu_data_to_ascii(imu_data_t * imu, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	char * ptr;
	*size = snprintf(dest, 128, "%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
			imu->timestamp,
			imu->accelerometer.x,
			imu->accelerometer.y,
			imu->accelerometer.z,
			imu->gyroscope.x,
			imu->gyroscope.y,
			imu->gyroscope.z,
			imu->magnetometer.x,
			imu->magnetometer.y,
			imu->magnetometer.z,
			imu->rotVectors.real,
			imu->rotVectors.i,
			imu->rotVectors.j,
			imu->rotVectors.k);

	return size;
}

int imu_data_to_ascii_no_timestamp(imu_data_t * imu, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	char * ptr;
	*size = snprintf(dest, 128, "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
			imu->accelerometer.x,
			imu->accelerometer.y,
			imu->accelerometer.z,
			imu->gyroscope.x,
			imu->gyroscope.y,
			imu->gyroscope.z,
			imu->magnetometer.x,
			imu->magnetometer.y,
			imu->magnetometer.z,
			imu->rotVectors.real,
			imu->rotVectors.i,
			imu->rotVectors.j,
			imu->rotVectors.k);

	return size;
}

int imu_data_to_bin(imu_data_t * imu, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	/* Write the data part */
	dest[0] = (unsigned char) (imu->timestamp >> 24);
	dest[1] = (unsigned char) (imu->timestamp >> 16);
	dest[2] = (unsigned char) (imu->timestamp >> 8);
	dest[3] = (unsigned char) (imu->timestamp);

	float_to_byte_array_msb(imu->accelerometer.x, &dest[4]);
	float_to_byte_array_msb(imu->accelerometer.y, &dest[7]);
	float_to_byte_array_msb(imu->accelerometer.z, &dest[11]);

	float_to_byte_array_msb(imu->gyroscope.x, &dest[15]);
	float_to_byte_array_msb(imu->gyroscope.y, &dest[19]);
	float_to_byte_array_msb(imu->gyroscope.z, &dest[23]);

	float_to_byte_array_msb(imu->magnetometer.x, &dest[27]);
	float_to_byte_array_msb(imu->magnetometer.y, &dest[31]);
	float_to_byte_array_msb(imu->magnetometer.z, &dest[35]);

	float_to_byte_array_msb(imu->rotVectors.real, &dest[39]);
	float_to_byte_array_msb(imu->rotVectors.i, &dest[43]);
	float_to_byte_array_msb(imu->rotVectors.j, &dest[47]);
	float_to_byte_array_msb(imu->rotVectors.k, &dest[51]);

	*size = 56;

	return 56;
}

int imu_data_to_bin_no_timestamp(imu_data_t * imu, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	float_to_byte_array_msb(imu->accelerometer.x, &dest[0]);
	float_to_byte_array_msb(imu->accelerometer.y, &dest[4]);
	float_to_byte_array_msb(imu->accelerometer.z, &dest[8]);

	float_to_byte_array_msb(imu->gyroscope.x, &dest[12]);
	float_to_byte_array_msb(imu->gyroscope.y, &dest[16]);
	float_to_byte_array_msb(imu->gyroscope.z, &dest[20]);

	float_to_byte_array_msb(imu->magnetometer.x, &dest[24]);
	float_to_byte_array_msb(imu->magnetometer.y, &dest[28]);
	float_to_byte_array_msb(imu->magnetometer.z, &dest[32]);

	float_to_byte_array_msb(imu->rotVectors.real, &dest[36]);
	float_to_byte_array_msb(imu->rotVectors.i, &dest[40]);
	float_to_byte_array_msb(imu->rotVectors.j, &dest[44]);
	float_to_byte_array_msb(imu->rotVectors.k, &dest[48]);

	*size = 52;

	return 52;
}

int pwc_data_to_bin(pwc_data_t * pwc, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	/* Write the data part */
	dest[0] = (char) (pwc->timestamp >> 24);
	dest[1] = (char) (pwc->timestamp >> 16);
	dest[2] = (char) (pwc->timestamp >> 8);
	dest[3] = (char) (pwc->timestamp);

	dest[4] = (char) (pwc->turn);
	dest[5] = (char) (pwc->speed);
	dest[6] = (char) (pwc->profile);
	dest[7] = (char) (pwc->mode);

	*size = 8;

	return 8;
}

/* TODO: for all the functions, also try the data pointer */
int pwc_data_to_bin_no_timestamp(pwc_data_t * pwc, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	/* Write the data part */
	dest[0] = (char) (pwc->turn);
	dest[1] = (char) (pwc->speed);
	dest[2] = (char) (pwc->profile);
	dest[3] = (char) (pwc->mode);

	*size = 4;

	return 4;
}

int pwc_data_to_bin_no_mode_no_timestamp(pwc_data_t * pwc, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	/* Write the data part */
	dest[0] = (char) (pwc->turn);
	dest[1] = (char) (pwc->speed);
	dest[2] = (char) (pwc->profile);

	*size = 3;

	return 3;
}


int pwc_data_to_ascii(pwc_data_t * pwc, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	switch (pwc->pwcType)
	{
		case PWC_TYPE_DX:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d\n",
					pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile);
			break;
		}
		case PWC_TYPE_PG:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d,%d\n",
					(unsigned int) pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile,
					pwc->mode);
			break;
		}
		case PWC_TYPE_LINX:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d,%d\n",
					pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile,
					pwc->mode);
			break;
		}
		default:
		{
			/* Error */
			return 0;
			break;
		}
	}
	return *size;
}

int pwc_data_to_ascii_no_timestamp(pwc_data_t * pwc, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	switch (pwc->pwcType)
	{
		case PWC_TYPE_DX:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d\n",
					pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile);
			break;
		}
		case PWC_TYPE_PG:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d,%d\n",
					pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile,
					pwc->mode);
			break;
		}
		case PWC_TYPE_LINX:
		{
			char * ptr;
			*size = snprintf(dest, 128, "%d,%d,%d,%d,%d\n",
					pwc->timestamp,
					pwc->turn,
					pwc->speed,
					pwc->profile,
					pwc->mode);
			break;
		}
		default:
		{
			/* Error */
			return 0;
			break;
		}
	}
	return *size;
}



int gps_data_to_bin_no_timestamp(gps_data_t * gps, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	/* Write the data part */
	/* TODO */
	*size = 4;

	return 4;
}

/* The following function parses in binary data this way:
 * pwcType|
 * TODO next: implement the distance sensor node parse
 */
int all_data_to_binary(data_container_t * container, unsigned char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	if (container == NULL)
	{
		return 0;
	}

	size_t p = 0;
	size_t tmp;

	p = rightshifty32_unsigned_integer_msb(container->timestamp, &dest[0]);
	dest[p++] = container->pwc.pwcType;
	p += pwc_data_to_bin_no_timestamp(&container->pwc, &dest[p], &tmp);
	p += imu_data_to_bin_no_timestamp(&container->imu, &dest[p], &tmp);

	*size = p;
	return p;


}

int all_data_to_ascii(data_container_t * container, unsigned char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	if (container == NULL)
	{
		return 0;
	}

	size_t p = 0;
	size_t tmp;
#if 0 //TODO: Pointer properly
	unsigned char ptr = NULL;
	ptr = dest;
	p= snprintf(dest, 128, "%d", container->timestamp);
	ptr += p;
	p= pwc_data_to_ascii_no_timestamp(&container->pwc, ptr, &tmp);
	ptr += p;
	dest[p++] = ',';
	ptr += 1;
	p= imu_data_to_ascii_no_timestamp(&container->imu, (dest+p), &tmp);
	ptr += 1;
	dest[ptr] = '\n';
#endif
	*size = snprintf(dest, 128, "%d,%d,%d,%d,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
			container->timestamp,
			container->pwc.turn,
			container->pwc.speed,
			container->pwc.profile,
			container->pwc.mode,
			container->imu.accelerometer.x,
			container->imu.accelerometer.y,
			container->imu.accelerometer.z,
			container->imu.gyroscope.x,
			container->imu.gyroscope.y,
			container->imu.gyroscope.z,
			container->imu.magnetometer.x,
			container->imu.magnetometer.y,
			container->imu.magnetometer.z,
			container->imu.rotVectors.real,
			container->imu.rotVectors.i,
			container->imu.rotVectors.j,
			container->imu.rotVectors.k);

	return *size;
}


/* This function returns 0 if the pointer is incorrect.
 * This function returns 26 if everything occured correctly.
 * 26 is also the number of bytes written in the array
 */

int imu9r_data_to_int16_t_scaled(imu_data_t imu, unsigned int scale, unsigned char *dest)
{
	/* Try the pointers */
	if (dest == NULL)
	{
		return 0;
	}

	imu.accelerometer.x *= scale;
	imu.accelerometer.y *= scale;
	imu.accelerometer.z *= scale;
	imu.gyroscope.x     *= scale;
	imu.gyroscope.y     *= scale;
	imu.gyroscope.z     *= scale;
	imu.rotVectors.real *= scale;
	imu.rotVectors.i    *= scale;
	imu.rotVectors.j    *= scale;
	imu.rotVectors.k    *= scale;

	unsigned short tmp = 0; /* Temporary variable used for the typecast and shift */

	tmp = (unsigned short) imu.accelerometer.x;
	dest[0] = (unsigned char) tmp << 8;
	dest[1] = (unsigned char) tmp;
	tmp = (unsigned short) imu.accelerometer.y;
	dest[2] = (unsigned char) tmp << 8;
	dest[3] = (unsigned char) tmp;
	tmp = (unsigned short) imu.accelerometer.z;
	dest[4] = (unsigned char) tmp << 8;
	dest[5] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.x;
	dest[6] = (unsigned char) tmp << 8;
	dest[7] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.y;
	dest[8] = (unsigned char) tmp << 8;
	dest[9] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.z;
	dest[10] = (unsigned char) tmp << 8;
	dest[11] = (unsigned char) tmp;
	tmp = (unsigned short) imu.magnetometer.x;
	dest[12] = (unsigned char) tmp << 8;
	dest[13] = (unsigned char) tmp;
	tmp = (unsigned short) imu.magnetometer.y;
	dest[14] = (unsigned char) tmp << 8;
	dest[15] = (unsigned char) tmp;
	tmp = (unsigned short) imu.magnetometer.z;
	dest[16] = (unsigned char) tmp << 8;
	dest[17] = (unsigned char) tmp;
	tmp = (unsigned short) imu.rotVectors.real;
	dest[18] = (unsigned char) tmp << 8;
	dest[19] = (unsigned char) tmp;
	tmp = (unsigned short) imu.rotVectors.i;
	dest[20] = (unsigned char) tmp << 8;
	dest[21] = (unsigned char) tmp;
	tmp = (unsigned short) imu.rotVectors.j;
	dest[22] = (unsigned char) tmp << 8;
	dest[23] = (unsigned char) tmp;
	tmp = (unsigned short) imu.rotVectors.k;
	dest[24] = (unsigned char) tmp << 8;
	dest[25] = (unsigned char) tmp;

	return 26;
}

/* This function returns 0 if the pointer is incorrect.
 * This function returns 26 if everything occured correctly.
 * 26 is also the number of bytes written in the array
 */
int imu6_data_to_int16_t_scaled(imu_data_t imu, unsigned int scale, unsigned char *dest)
{
	/* Try the pointers */
	if (dest == NULL)
	{
		return 0;
	}

	imu.accelerometer.x *= scale;
	imu.accelerometer.y *= scale;
	imu.accelerometer.z *= scale;
	imu.gyroscope.x     *= scale;
	imu.gyroscope.y     *= scale;
	imu.gyroscope.z     *= scale;

	unsigned short tmp = 0; /* Temporary variable used for the typecast and shift */

	tmp = (unsigned short) imu.accelerometer.x;
	dest[0] = (unsigned char) tmp << 8;
	dest[1] = (unsigned char) tmp;
	tmp = (unsigned short) imu.accelerometer.y;
	dest[2] = (unsigned char) tmp << 8;
	dest[3] = (unsigned char) tmp;
	tmp = (unsigned short) imu.accelerometer.z;
	dest[4] = (unsigned char) tmp << 8;
	dest[5] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.x;
	dest[6] = (unsigned char) tmp << 8;
	dest[7] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.y;
	dest[8] = (unsigned char) tmp << 8;
	dest[9] = (unsigned char) tmp;
	tmp = (unsigned short) imu.gyroscope.z;
	dest[10] = (unsigned char) tmp << 8;
	dest[11] = (unsigned char) tmp;

	return 12;
}

/* todo: move to a specific file */


#include <app_init.h>

int imu_data_log(imu_data_t * imu, char * dest, size_t * size)
{
	if ((dest == NULL) || (size == NULL))
	{
		return 0;
	}
	char * ptr;
	char hour, min, sec;
	char year, month, day;
	unsigned int sub, fraction;
	unsigned long epoch;

	//getTime(&hour, &min, &sec);
	getTimeWithSubs(&hour, &min, &sec, &sub, &fraction);
	getDate(&year, &month, &day);

	struct tm dateTime;
	memset(&dateTime, 0, sizeof(dateTime));
	dateTime.tm_sec = sec;
	dateTime.tm_min = min;
	dateTime.tm_hour = hour;
	dateTime.tm_mday = day;
	dateTime.tm_mon = month - 1;
	dateTime.tm_year = year + 100; /* Year - 1900 */

	uint64_t T1 = (HAL_GetTick()/1000) + epochBootTime;
	uint64_t ms =  HAL_GetTick() - (T1*1000) + (epochBootTime*1000);

	epoch = mktime(&dateTime);
	unsigned int H = HAL_GetTick();
	unsigned int msecs = (epochBootTime + (HAL_GetTick()/1000)) - epoch;
	msecs = HAL_GetTick() - msecs;

    //printf("[%d] Its %d/%d/%d - %d:%d:%d.%d\n", HAL_GetTick(), year, month, day, hour, min, sec, ms);


	*size = snprintf(dest, 128, "imu,%d:%d:%d.%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
			hour, min, sec, ms,
			//imu->timestamp,
			imu->accelerometer.x,
			imu->accelerometer.y,
			imu->accelerometer.z,
			imu->gyroscope.x,
			imu->gyroscope.y,
			imu->gyroscope.z,
			imu->magnetometer.x,
			imu->magnetometer.y,
			imu->magnetometer.z,
			imu->rotVectors.real,
			imu->rotVectors.i,
			imu->rotVectors.j,
			imu->rotVectors.k);

	return size;
}

/* The part following is up to date with our protocol's specifications (see https://educat2seas.eu/data/api/setup/parameter/33/info)*/
int datatypeToRaw_joystick_dx2(pwc_data_t * pData, unsigned char *dest)
{
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) (pData->estimatedSpeed >> 8);
	dest[1] = (unsigned char) pData->estimatedSpeed;
	dest[2] = (unsigned char) pData->turn;
	dest[3] = (unsigned char) pData->speed;
	dest[4] = (unsigned char) pData->profile;

	return 5;
}

int datatypeToRaw_joystick_pg(pwc_data_t * pData, unsigned char *dest)
{
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) (pData->estimatedSpeed >> 8);
	dest[1] = (unsigned char) pData->estimatedSpeed;
	dest[2] = (unsigned char) pData->turn;
	dest[3] = (unsigned char) pData->speed;
	dest[4] = (unsigned char) pData->profile;
	dest[5] = (unsigned char) pData->mode;

#if PRINTF_OPERATIONS_DBG
	sprintf(string, "%u [operations] [datatypeToRaw_joystick_pg] [dest 0-7] 0x%2X %2X %2X %2X %2X %2X\n",
												(unsigned int) HAL_GetTick(),
												dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);
    xQueueSend(pPrintQueue, string, 0);
#endif

	return 6;
}

int datatypeToRaw_joystick_linx(pwc_data_t * pData, unsigned char *dest)
{
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) (pData->estimatedSpeed >> 8);
	dest[1] = (unsigned char) pData->estimatedSpeed;
	dest[2] = (unsigned char) pData->turn;
	dest[3] = (unsigned char) pData->speed;
	dest[4] = (unsigned char) pData->profile;

	return 5;
}

int datatypeToRaw_RTC(uint64_t * pData, unsigned char *dest)
{
  if (!dest || !pData)
  { /* Try the pointers */
	return 0;
  }
  dest[0] = *pData >> 56;
  dest[1] = *pData >> 48;
  dest[2] = *pData >> 40;
  dest[3] = *pData >> 32;
  dest[4] = *pData >> 24;
  dest[5] = *pData >> 16;
  dest[6] = *pData >> 8;
  dest[7] = *pData;
  return 8;
}


int datatypeToRaw_imu_9axis_rot_vec(imu_data_t * pImu, unsigned char *dest)
{
	if (!dest || !pImu)
	{ /* Try the pointers */
		return 0;
	}
	imu_data_t imu;
	unsigned int scale = 100;
	unsigned int rotScale = 1000;
	memset(&imu, 0, sizeof(imu_data_t));
	imu = *pImu; // Copy data as we will modify it
	imu.accelerometer.x *= scale;
	imu.accelerometer.y *= scale;
	imu.accelerometer.z *= scale;
	imu.gyroscope.x     *= scale;
	imu.gyroscope.y     *= scale;
	imu.gyroscope.z     *= scale;
	imu.rotVectors.real *= rotScale;
	imu.rotVectors.i    *= rotScale;
	imu.rotVectors.j    *= rotScale;
	imu.rotVectors.k    *= rotScale;
	short tmp = 0; /* Temporary variable used for the typecast and shift */
	// in future, use the folowing union
#if 0
	union {
		short tmp;
		unsigned char bytes[2];
	} u;
	memset(&u, 0, sizeof(u));
#endif
	tmp = (short) imu.accelerometer.x;
	dest[0] =  (tmp >> 8) & 0xFF;
	dest[1] =  (tmp) & 0xFF;
	tmp = (short) imu.accelerometer.y;
	dest[2] =  (tmp >> 8) & 0xFF;
	dest[3] =  tmp & 0xFF;
	tmp = (short) imu.accelerometer.z;
	dest[4] = tmp >> 8 & 0xFF;
	dest[5] = tmp & 0xFF;
	tmp = (short) imu.gyroscope.x;
	dest[6] =  tmp >> 8 & 0xFF;
	dest[7] = tmp & 0xFF;
	tmp = (short) imu.gyroscope.y;
	dest[8] =  tmp >> 8 & 0xFF;
	dest[9] =  tmp & 0xFF;
	tmp = (short) imu.gyroscope.z;
	dest[10] =  tmp >> 8 & 0xFF;
	dest[11] =  tmp & 0xFF;
	tmp = (short) imu.magnetometer.x;
	dest[12] =  tmp >> 8 & 0xFF;
	dest[13] =  tmp & 0xFF;
	tmp = (short) imu.magnetometer.y;
	dest[14] =  tmp >> 8 & 0xFF;
	dest[15] = tmp & 0xFF;
	tmp = (short) imu.magnetometer.z;
	dest[16] = tmp >> 8 & 0xFF;
	dest[17] = tmp & 0xFF;
	tmp = (short) imu.rotVectors.real;
	dest[18] = tmp >> 8 & 0xFF;
	dest[19] = tmp & 0xFF;
	tmp = (short) imu.rotVectors.i;
	dest[20] =  tmp >> 8 & 0xFF;
	dest[21] =  tmp & 0xFF;
	tmp = ( short) imu.rotVectors.j;
	dest[22] =  tmp >> 8 & 0xFF;
	dest[23] =  tmp & 0xFF;
	tmp = ( short) imu.rotVectors.k;
	dest[24] =  tmp >> 8 & 0xFF;
	dest[25] =  tmp & 0xFF;
	return 26;
}

int datatypeToRaw_imu_6axis(imu_data_t * pImu, unsigned int scale, unsigned char *dest)
{
  if (!dest || !pImu)
  { /* Try the pointers */
	return 0;
  }
  imu_data_t imu;
  memset(&imu, 0, sizeof(imu_data_t));
  imu = *pImu; // Copy data as we will modify it
  imu.accelerometer.x *= scale;
  imu.accelerometer.y *= scale;
  imu.accelerometer.z *= scale;
  imu.gyroscope.x     *= scale;
  imu.gyroscope.y     *= scale;
  imu.gyroscope.z     *= scale;
  unsigned short tmp = 0; /* Temporary variable used for the typecast and shift */
  tmp = (unsigned short) imu.accelerometer.x;
  dest[0] = (unsigned char) tmp >> 8 & 0xFF;
  dest[1] = (unsigned char) tmp & 0xFF;
  tmp = (unsigned short) imu.accelerometer.y;
  dest[2] = (unsigned char) tmp >> 8 & 0xFF;
  dest[3] = (unsigned char) tmp & 0xFF;
  tmp = (unsigned short) imu.accelerometer.z;
  dest[4] = (unsigned char) tmp >> 8 & 0xFF;
  dest[5] = (unsigned char) tmp & 0xFF;
  tmp = (unsigned short) imu.gyroscope.x;
  dest[6] = (unsigned char) tmp >> 8 & 0xFF;
  dest[7] = (unsigned char) tmp & 0xFF;
  tmp = (unsigned short) imu.gyroscope.y;
  dest[8] = (unsigned char) tmp >> 8 & 0xFF;
  dest[9] = (unsigned char) tmp & 0xFF;
  tmp = (unsigned short) imu.gyroscope.z;
  dest[10] = (unsigned char) tmp >> 8 & 0xFF;
  dest[11] = (unsigned char) tmp & 0xFF;
  return 12;
}

int datatypeToRaw_imu_quat(imu_100Hz_data_t * pImu, unsigned char *dest)
{
  if (!dest || !pImu)
  { /* Try the pointers */
	return 0;
  }
  imu_100Hz_data_t imu;
  unsigned int scale = 1;
  unsigned int rotScale = 1;
  memset(&imu, 0, sizeof(imu_100Hz_data_t));
  imu = *pImu; // Copy data as we will modify it
//  Only Quaternions will be used:
//	imu.accelerometer.x *= scale;
//	imu.accelerometer.y *= scale;
//	imu.accelerometer.z *= scale;
//	imu.gyroscope.x     *= scale;
//	imu.gyroscope.y     *= scale;
//	imu.gyroscope.z     *= scale;
  imu.rotVectors1.real *= rotScale;
  imu.rotVectors1.i    *= rotScale;
  imu.rotVectors1.j    *= rotScale;
  imu.rotVectors1.k    *= rotScale;
  short tmp = 0; /* Temporary variable used for the type cast and shift */
//	tmp = (short) imu.accelerometer.x;
//	dest[0] =  (tmp >> 8) & 0xFF;
//	dest[1] =  (tmp) & 0xFF;
//	tmp = (short) imu.accelerometer.y;
//	dest[2] =  (tmp >> 8) & 0xFF;
//	dest[3] =  tmp & 0xFF;
//	tmp = (short) imu.accelerometer.z;
//	dest[4] = tmp >> 8 & 0xFF;
//	dest[5] = tmp & 0xFF;
//	tmp = (short) imu.gyroscope.x;
//	dest[6] =  tmp >> 8 & 0xFF;
//	dest[7] = tmp & 0xFF;
//	tmp = (short) imu.gyroscope.y;
//	dest[8] =  tmp >> 8 & 0xFF;
//	dest[9] =  tmp & 0xFF;
//	tmp = (short) imu.gyroscope.z;
//	dest[10] =  tmp >> 8 & 0xFF;
//	dest[11] =  tmp & 0xFF;
//	tmp = (short) imu.magnetometer.x;
//	dest[12] =  tmp >> 8 & 0xFF;
//	dest[13] =  tmp & 0xFF;
//	tmp = (short) imu.magnetometer.y;
//	dest[14] =  tmp >> 8 & 0xFF;
//	dest[15] = tmp & 0xFF;
//	tmp = (short) imu.magnetometer.z;
//	dest[16] = tmp >> 8 & 0xFF;
//	dest[17] = tmp & 0xFF;
// Only Quaternions will be used:
	tmp = (short) imu.rotVectors1.real;
	dest[0] = tmp >> 8 & 0xFF;
	dest[1] = tmp & 0xFF;
	tmp = (short) imu.rotVectors1.i;
	dest[2] = tmp >> 8 & 0xFF;
	dest[3] = tmp & 0xFF;
	tmp = (short) imu.rotVectors1.j;
	dest[4] = tmp >> 8 & 0xFF;
	dest[5] = tmp & 0xFF;
	tmp = (short) imu.rotVectors1.k;
	dest[6] = tmp >> 8 & 0xFF;
	dest[7] = tmp & 0xFF;
//#if PRINTF_OPERATIONS_DBG
//	sprintf(string, "%u [operations] [datatypeToRaw_imu_quat] [dest 0-7] 0x%0X %0X %0X %0X %0X %0X %0X %0X\n",
//												(unsigned int) HAL_GetTick(),
//												dest[0],dest[1],dest[2],dest[3],dest[4],dest[5],dest[6],dest[7]);
//  xQueueSend(pPrintQueue, string, 0);
//#endif
  return 8;
}

int datatypeToRaw_imu_quat_gyro_acc(imu_100Hz_data_t * pImu, unsigned char *dest)
{
  if (!dest || !pImu)
  { /* Try the pointers */
	return 0;
  }
  imu_100Hz_data_t imu;
  unsigned int scale = 1;
  unsigned int rotScale = 1;
  memset(&imu, 0, sizeof(imu_100Hz_data_t));
  imu = *pImu; // Copy data as we will modify it
  imu.rotVectors1.real *= rotScale;
  imu.rotVectors1.i    *= rotScale;
  imu.rotVectors1.j    *= rotScale;
  imu.rotVectors1.k    *= rotScale;
  imu.gyroscope1.x     *= scale;
  imu.gyroscope1.y     *= scale;
  imu.gyroscope1.z     *= scale;
  imu.accelerometer1.x *= scale;
  imu.accelerometer1.y *= scale;
  imu.accelerometer1.z *= scale;
  short tmp = 0; /* Temporary variable used for the type cast and shift */
  tmp = (short) imu.rotVectors1.real;
  dest[0] = tmp >> 8 & 0xFF;
  dest[1] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.i;
  dest[2] = tmp >> 8 & 0xFF;
  dest[3] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.j;
  dest[4] = tmp >> 8 & 0xFF;
  dest[5] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.k;
  dest[6] = tmp >> 8 & 0xFF;
  dest[7] = tmp & 0xFF;
  tmp = (short) imu.gyroscope1.x;
  dest[8] =  tmp >> 8 & 0xFF;
  dest[9] = tmp & 0xFF;
  tmp = (short) imu.gyroscope1.y;
  dest[10] =  tmp >> 8 & 0xFF;
  dest[11] =  tmp & 0xFF;
  tmp = (short) imu.gyroscope1.z;
  dest[12] =  tmp >> 8 & 0xFF;
  dest[13] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer1.x;
  dest[14] =  (tmp >> 8) & 0xFF;
  dest[15] =  (tmp) & 0xFF;
  tmp = (short) imu.accelerometer1.y;
  dest[16] =  (tmp >> 8) & 0xFF;
  dest[17] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer1.z;
  dest[18] = tmp >> 8 & 0xFF;
  dest[19] = tmp & 0xFF;
//  tmp = (short) imu.magnetometer.x;
//  dest[12] =  tmp >> 8 & 0xFF;
//  dest[13] =  tmp & 0xFF;
//  tmp = (short) imu.magnetometer.y;
//  dest[14] =  tmp >> 8 & 0xFF;
//  dest[15] = tmp & 0xFF;
//  tmp = (short) imu.magnetometer.z;
//  dest[16] = tmp >> 8 & 0xFF;
//  dest[17] = tmp & 0xFF;
//#if PRINTF_OPERATIONS_DBG
//	sprintf(string, "%u [operations] [datatypeToRaw_imu_quat] [dest 0-7] 0x%0X %0X %0X %0X %0X %0X %0X %0X\n",
//												(unsigned int) HAL_GetTick(),
//												dest[0],dest[1],dest[2],dest[3],dest[4],dest[5],dest[6],dest[7]);
//  xQueueSend(pPrintQueue, string, 0);
//#endif
  return 20;
}

int datatypeToRaw_imu_quat_gyro_acc_100Hz(imu_100Hz_data_t * pImu, unsigned char *dest)
{
  if (!dest || !pImu)
  { /* Try the pointers */
	return 0;
  }
  imu_100Hz_data_t imu;
  unsigned int scale = 1;
  unsigned int rotScale = 1;
  memset(&imu, 0, sizeof(imu_100Hz_data_t));
  imu = *pImu; // Copy data as we will modify it
  imu.rotVectors1.real *= rotScale;
  imu.rotVectors1.i    *= rotScale;
  imu.rotVectors1.j    *= rotScale;
  imu.rotVectors1.k    *= rotScale;
  imu.gyroscope1.x     *= scale;
  imu.gyroscope1.y     *= scale;
  imu.gyroscope1.z     *= scale;
  imu.accelerometer1.x *= scale;
  imu.accelerometer1.y *= scale;
  imu.accelerometer1.z *= scale;
  imu.rotVectors2.real *= rotScale;
  imu.rotVectors2.i    *= rotScale;
  imu.rotVectors2.j    *= rotScale;
  imu.rotVectors2.k    *= rotScale;
  imu.gyroscope2.x     *= scale;
  imu.gyroscope2.y     *= scale;
  imu.gyroscope2.z     *= scale;
  imu.accelerometer2.x *= scale;
  imu.accelerometer2.y *= scale;
  imu.accelerometer2.z *= scale;
  short tmp = 0; /* Temporary variable used for the type cast and shift */
  tmp = (short) imu.rotVectors1.real;
  dest[0] = tmp >> 8 & 0xFF;
  dest[1] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.i;
  dest[2] = tmp >> 8 & 0xFF;
  dest[3] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.j;
  dest[4] = tmp >> 8 & 0xFF;
  dest[5] = tmp & 0xFF;
  tmp = (short) imu.rotVectors1.k;
  dest[6] = tmp >> 8 & 0xFF;
  dest[7] = tmp & 0xFF;
  tmp = (short) imu.gyroscope1.x;
  dest[8] =  tmp >> 8 & 0xFF;
  dest[9] = tmp & 0xFF;
  tmp = (short) imu.gyroscope1.y;
  dest[10] =  tmp >> 8 & 0xFF;
  dest[11] =  tmp & 0xFF;
  tmp = (short) imu.gyroscope1.z;
  dest[12] =  tmp >> 8 & 0xFF;
  dest[13] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer1.x;
  dest[14] =  (tmp >> 8) & 0xFF;
  dest[15] =  (tmp) & 0xFF;
  tmp = (short) imu.accelerometer1.y;
  dest[16] =  (tmp >> 8) & 0xFF;
  dest[17] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer1.z;
  dest[18] = tmp >> 8 & 0xFF;
  dest[19] = tmp & 0xFF;
  tmp = (short) imu.rotVectors2.real;
  dest[20] = tmp >> 8 & 0xFF;
  dest[21] = tmp & 0xFF;
  tmp = (short) imu.rotVectors2.i;
  dest[22] = tmp >> 8 & 0xFF;
  dest[23] = tmp & 0xFF;
  tmp = (short) imu.rotVectors2.j;
  dest[24] = tmp >> 8 & 0xFF;
  dest[25] = tmp & 0xFF;
  tmp = (short) imu.rotVectors2.k;
  dest[26] = tmp >> 8 & 0xFF;
  dest[27] = tmp & 0xFF;
  tmp = (short) imu.gyroscope2.x;
  dest[28] =  tmp >> 8 & 0xFF;
  dest[29] = tmp & 0xFF;
  tmp = (short) imu.gyroscope2.y;
  dest[30] =  tmp >> 8 & 0xFF;
  dest[31] =  tmp & 0xFF;
  tmp = (short) imu.gyroscope2.z;
  dest[32] =  tmp >> 8 & 0xFF;
  dest[33] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer2.x;
  dest[34] =  (tmp >> 8) & 0xFF;
  dest[35] =  (tmp) & 0xFF;
  tmp = (short) imu.accelerometer2.y;
  dest[36] =  (tmp >> 8) & 0xFF;
  dest[37] =  tmp & 0xFF;
  tmp = (short) imu.accelerometer2.z;
  dest[38] = tmp >> 8 & 0xFF;
  dest[39] = tmp & 0xFF;
//#if PRINTF_OPERATIONS_DBG
//	sprintf(string, "%u [operations] [datatypeToRaw_imu_quat] [dest 0-7] 0x%0X %0X %0X %0X %0X %0X %0X %0X\n",
//												(unsigned int) HAL_GetTick(),
//												dest[0],dest[1],dest[2],dest[3],dest[4],dest[5],dest[6],dest[7]);
//  xQueueSend(pPrintQueue, string, 0);
//#endif
  return 40;
}

int datatypeToRaw_usbad_instrument(usbad_data_t *pData, unsigned char *dest)
{
  if(!dest || !pData)
  {/* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->OAScalculatedValue;
  dest[1] = (unsigned char) pData->feedbackStatue;
  return 2;
}

int datatypeToRaw_usbad_sensor_instrument(usbad_data_t *pData, unsigned char *dest)
{
  if(!dest || !pData)
  {/* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->OAScalculatedValue;
  dest[1] = (unsigned char) pData->feedbackStatue;
  dest[2] = (unsigned char) pData->sensorActivate;
  return 3;
}

int datatypeToRaw_canDistanceNodeD1(can_distance_node_d1 * pData, unsigned char *dest)
{
  if (!dest || !pData)
  {/* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->distance << 8;
  dest[1] = (unsigned char) pData->distance ;
  return 2;
}

int datatypeToRaw_canDistanceNodeD2(can_distance_node_d2 * pData, unsigned char *dest)
{
  if (!dest || !pData)
  { /* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->distance << 8;
  dest[1] = (unsigned char) pData->distance ;
  return 2;
}

int datatypeToRaw_canDistanceNodeD3(can_distance_node_d3 * pData, unsigned char *dest)
{
  if (!dest || !pData)
  { /* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->calculatedDistance << 8;
  dest[1] = (unsigned char) pData->calculatedDistance;
  dest[2] = (unsigned char) pData->distanceUS << 8;
  dest[3] = (unsigned char) pData->distanceUS;
  dest[4] = (unsigned char) pData->distanceIR;
  return 5;
}

int datatypeToRaw_canDistanceNodeD4(can_distance_node_d4 * pData, unsigned char *dest)
{
  if (!dest || !pData)
  { /* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->calculatedDistance << 8;
  dest[1] = (unsigned char) pData->calculatedDistance;
  dest[2] = (unsigned char) pData->distanceUS << 8;
  dest[3] = (unsigned char) pData->distanceUS;
  dest[4] = (unsigned char) pData->distanceIR1;
  dest[5] = (unsigned char) pData->distanceIR2;
#if PRINTF_OPERATIONS_DBG
  sprintf(string, "%u [operations] [datatypeToRaw_canDistanceNodeD4] [dest 0-7] 0x%2X %2X %2X %2X %2X %2X\n",
						(unsigned int) HAL_GetTick(), dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return 6;
}

int datatypeToRaw_canDistanceNodeD5(can_distance_node_d5 * pData, unsigned char *dest)
{
  if (!dest || !pData)
  { /* Try the pointers */
	return 0;
  }
  dest[0] = (unsigned char) pData->calculatedDistance << 8;
  dest[1] = (unsigned char) pData->calculatedDistance;
  dest[2] = (unsigned char) pData->distanceUS << 8;
  dest[3] = (unsigned char) pData->distanceUS;
  dest[4] = (unsigned char) pData->distanceIR1;
  dest[5] = (unsigned char) pData->distanceIR2;
  dest[6] = (unsigned char) pData->distanceIR3;
  return 7;
}

int datatypeToRaw_canDistanceNodeD6(can_distance_node_d6 * pData, unsigned char *dest)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) pData->calculatedDistance << 8;
	dest[1] = (unsigned char) pData->calculatedDistance;
	dest[3] = (unsigned char) pData->distanceIR1;
	dest[4] = (unsigned char) pData->distanceIR2;
	dest[5] = (unsigned char) pData->distanceIR3;
	dest[6] = (unsigned char) pData->distanceIR4;

#if PRINTF_OPERATIONS_DBG
	sprintf(string, "%u [operations] [datatypeToRaw_canDistanceNodeD6] [dest 0-7] 0x%2X %2X %2X %2X %2X %2X\n",
												(unsigned int) HAL_GetTick(),
												dest[0],dest[1],dest[2],dest[3],dest[4],dest[5]);
    xQueueSend(pPrintQueue, string, 0);
#endif

	return 6;
}

int datatypeToRaw_canDistanceNodeD7(can_distance_node_d6 * pData, unsigned char *dest)
{
	/* Correspond to the can_distance_D6 but with only the calculated data */
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) (pData->calculatedDistance >> 8);
	dest[1] = (unsigned char) pData->calculatedDistance;

	return 2;
}

int datatypeToRaw_canDistanceNodeD8(can_distance_node_d5 * pData, unsigned char * pDest)
{
	if(pData == NULL)
		return 0;
	if(pDest == NULL)
		return 0;

	pDest[0] = (unsigned char) (pData->calculatedDistance >> 8);
	pDest[1] = (unsigned char) pData->calculatedDistance;

	return 2;
}

int datatypeToRaw_gpsMinData(gps_data_t * pData, unsigned char *dest)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	float_to_byte_array_msb(pData->lon, &dest[0]);
	float_to_byte_array_msb(pData->lat, &dest[4]);
	float_to_byte_array_msb(pData->hMSL, &dest[8]);
	float_to_byte_array_msb(pData->gSpeed, &dest[12]);
	dest[16] = pData->gnssFixOK;


	return 17;
}

/* The part following is up to date with our protocol's specifications (see https://educat2seas.eu/data/api/setup/parameter/33/info)*/
int datatypeToAscii_joystick_dx2(unsigned int instrumentType, pwc_data_t * pData, unsigned char *dest, unsigned int lineReturn)
{
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_joystick_dx2_MAX_OUTPUT, "DX2,%d,%d,%d,%d,%d\n",
																			instrumentType,
																			pData->estimatedSpeed, pData->turn,
																			pData->speed, pData->profile);
	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_joystick_dx2_MAX_OUTPUT, "DX2,%d,%d,%d,%d,%d",
																			instrumentType,
																			pData->estimatedSpeed, pData->turn,
																			pData->speed, pData->profile);
	}
	if (elems <=  datatypeToAscii_joystick_dx2_MAX_OUTPUT)
	{
		return elems;
	}
	return elems;
}

int datatypeToAscii_joystick_pg(unsigned int instrumentType, pwc_data_t * pData, unsigned char *dest, unsigned int lineReturn)
{
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_joystick_pg_MAX_OUTPUT, "PNG,%d,%d,%d,%d,%d,%d\n",
																			instrumentType,
																			pData->estimatedSpeed, pData->turn,
																			pData->speed, pData->profile,
																			pData->mode);
	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_joystick_pg_MAX_OUTPUT, "PNG,%d,%d,%d,%d,%d,%d",
																			instrumentType,
																			pData->estimatedSpeed, pData->turn,
																			pData->speed, pData->profile,
																			pData->mode);
	}
	if (elems <=  datatypeToAscii_joystick_pg_MAX_OUTPUT)
	{
		return elems;
	}
	return 0;
}

int datatypeToAscii_imu_9axis_rot_vec(unsigned int instrumentType, imu_data_t * pImu, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pImu)
	{
		return 0;
	}

	imu_data_t imu;
	unsigned int elems = 0;

	memset(&imu, 0, sizeof(imu_data_t));

	imu = *pImu; // Copy data as we will modify it
	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_imu_9axis_MAX_OUTPUT,
				"IMU,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
				//imu->timestamp,
				instrumentType,
				imu.accelerometer.x,
				imu.accelerometer.y,
				imu.accelerometer.z,
				imu.gyroscope.x,
				imu.gyroscope.y,
				imu.gyroscope.z,
				imu.magnetometer.x,
				imu.magnetometer.y,
				imu.magnetometer.z,
				imu.rotVectors.real,
				imu.rotVectors.i,
				imu.rotVectors.j,
				imu.rotVectors.k);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_imu_9axis_MAX_OUTPUT,
				"IMU,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
				//imu->timestamp,
				instrumentType,
				imu.accelerometer.x,
				imu.accelerometer.y,
				imu.accelerometer.z,
				imu.gyroscope.x,
				imu.gyroscope.y,
				imu.gyroscope.z,
				imu.magnetometer.x,
				imu.magnetometer.y,
				imu.magnetometer.z,
				imu.rotVectors.real,
				imu.rotVectors.i,
				imu.rotVectors.j,
				imu.rotVectors.k);

	}
	if (elems <=  datatypeToAscii_imu_9axis_MAX_OUTPUT)
	{
		return elems;
	}
	return 0;
}

int datatypeToAscii_imu_6axis(unsigned int instrumentType, imu_data_t * pImu, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pImu)
	{
		return 0;
	}

	imu_data_t imu;
	unsigned int elems = 0;

	memset(&imu, 0, sizeof(imu_data_t));

	imu = *pImu; // Copy data as we will modify it
	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_imu_6axis_MAX_OUTPUT,
				"IMU,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f\n",
				//imu->timestamp,
				instrumentType,
				imu.accelerometer.x,
				imu.accelerometer.y,
				imu.accelerometer.z,
				imu.gyroscope.x,
				imu.gyroscope.y,
				imu.gyroscope.z);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_imu_6axis_MAX_OUTPUT,
				"IMU,%d,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
				//imu->timestamp,
				instrumentType,
				imu.accelerometer.x,
				imu.accelerometer.y,
				imu.accelerometer.z,
				imu.gyroscope.x,
				imu.gyroscope.y,
				imu.gyroscope.z);

	}
	if (elems <=  datatypeToAscii_imu_6axis_MAX_OUTPUT)
	{
		return elems;
	}
	return 0;
}

int datatypeToAscii_canDistanceNodeD1(unsigned int instrumentType, can_distance_node_d1 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd1_MAX_OUTPUT,
				"CD1,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->distance);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd1_MAX_OUTPUT,
				"CD1,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->distance);

	}
	if (elems <=  datatypeToAscii_can_cd1_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}
int datatypeToAscii_canDistanceNodeD2(unsigned int instrumentType, can_distance_node_d2 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd2_MAX_OUTPUT,
				"CD2,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->distance);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd2_MAX_OUTPUT,
				"CD2,%d,%d,%d",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->distance);

	}
	if (elems <=  datatypeToAscii_can_cd2_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}


int datatypeToAscii_canDistanceNodeD3(unsigned int instrumentType, can_distance_node_d3 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	dest[0] = (unsigned char) pData->calculatedDistance << 8;
	dest[1] = (unsigned char) pData->calculatedDistance;
	dest[2] = (unsigned char) pData->distanceUS << 8;
	dest[3] = (unsigned char) pData->distanceUS;
	dest[4] = (unsigned char) pData->distanceIR;

	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd3_MAX_OUTPUT,
				"CD3,%d,%d,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd3_MAX_OUTPUT,
				"CD3,%d,%d,%d,%d,%d",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR);

	}
	if (elems <=  datatypeToAscii_can_cd3_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}

int datatypeToAscii_canDistanceNodeD4(unsigned int instrumentType, can_distance_node_d4 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd4_MAX_OUTPUT,
				"CD4,%d,%d,%d,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR1,
				pData->distanceIR2);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd4_MAX_OUTPUT,
				"CD4,%d,%d,%d,%d,%d,%d",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR1,
				pData->distanceIR2);
	}
	if (elems <=  datatypeToAscii_can_cd4_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}

int datatypeToAscii_canDistanceNodeD5(unsigned int instrumentType, can_distance_node_d5 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd5_MAX_OUTPUT,
				"CD5,%d,%d,%d,%d,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR1,
				pData->distanceIR2,
				pData->distanceIR3);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd5_MAX_OUTPUT,
				"CD5,%d,%d,%d,%d,%d,%d,%d",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceUS,
				pData->distanceIR1,
				pData->distanceIR2,
				pData->distanceIR3);
	}
	if (elems <=  datatypeToAscii_can_cd5_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}


int datatypeToAscii_canDistanceNodeD6(unsigned int instrumentType, can_distance_node_d6 * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_can_cd6_MAX_OUTPUT,
				"CD6,%d,%d,%d,%d,%d,%d,%d\n",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceIR1,
				pData->distanceIR2,
				pData->distanceIR3,
				pData->distanceIR4);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_can_cd6_MAX_OUTPUT,
				"CD6,%d,%d,%d,%d,%d,%d,%d",
				//imu->timestamp,
				instrumentType,
				pData->ID,
				pData->calculatedDistance,
				pData->distanceIR1,
				pData->distanceIR2,
				pData->distanceIR3,
				pData->distanceIR4);
	}
	if (elems <=  datatypeToAscii_can_cd5_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}


int datatypeToAscii_gpsMinData(unsigned int instrumentType, gps_data_t * pData, unsigned char *dest, unsigned int lineReturn)
{
	/* Try the pointers */
	if (!dest || !pData)
	{
		return 0;
	}

	unsigned int elems = 0;

	if (lineReturn)
	{
		elems = snprintf(dest, datatypeToAscii_gps_minData_MAX_OUTPUT,
				"GPS,%d,%.8f,%.8f,%.2f,%.2f\n",
				//imu->timestamp,
				instrumentType,
				pData->lon,
				pData->lat,
				pData->hMSL,
				pData->gSpeed);

	}
	else
	{
		elems = snprintf(dest, datatypeToAscii_gps_minData_MAX_OUTPUT,
				"GPS,%d,%.8f,%.8f,%.2f,%.2f",
				//imu->timestamp,
				instrumentType,
				pData->lon,
				pData->lat,
				pData->hMSL,
				pData->gSpeed);
	}
	if (elems <=  datatypeToAscii_gps_minData_MAX_OUTPUT)
	{
		return elems;
	}

	return 0;
}

int rawToStruct_joystick_dx2(unsigned char *source, unsigned int size, unsigned int n_elem, pwc_data_t * pData)
{
	if(!source || !pData)
	{
		return 0;
	}

	pData->estimatedSpeed = source[0] << 8 | source[1];
	pData->turn = source[2];
	pData->speed = source[3];
	pData->profile = source[4];

	return 5;
}


int rawToStruct_joystick_pg(unsigned char * source, unsigned int size, unsigned int n_elem, pwc_data_t * pData)
{
	if(!source || !pData)
	{
		return 0;
	}
	pData->estimatedSpeed = source[0] << 8 | source[1];
	pData->turn = source[2];
	pData->speed = source[3];
	pData->profile = source[4];
	pData->mode = source[6];

#if PRINTF_OPERATIONS_DBG
	sprintf(string, "%u [operations] [rawToStruct_joystick_pg]\n", (unsigned int) HAL_GetTick());
    xQueueSend(pPrintQueue, string, 0);
#endif


	return 6;
}
int rawToStruct_joystick_linx(unsigned char * source, unsigned int size, unsigned int n_elem, pwc_data_t * pData)
{
	if(!source || !pData)
	{
		return 0;
	}

	pData->estimatedSpeed = source[0] << 8 | source[1];
	pData->turn = source[2];
	pData->speed = source[3];
	pData->profile = source[4];

	return 5;
}

int rawToStruct_RTC(unsigned char * source, unsigned int size, unsigned int n_elem, uint64_t * pData)
{
	if(!source || !pData)
	{
		return 0;
	}

	*pData = source[0] << 56 | source[1] << 48 | source[2] << 40 | source[3] << 32 | source[4] << 24 | source[5] << 16 | source[6] << 8 | source[7];

	return 8;
}

int rawToStruct_imu_9axis_rot_vec(unsigned int * source, unsigned int size, unsigned int n_elem, imu_data_t * pData)
{
	if(!source || !pData)
	{
		return 0;
	}

	const int scale = 100;
	const int rotScale = 1000;

	pData->accelerometer.x = source[0] << 8 | source[1];
	pData->accelerometer.y = source[2] << 8 | source[3];
	pData->accelerometer.z = source[4] << 8 | source[5];
	pData->gyroscope.x = source[6] << 8 | source[7];
	pData->gyroscope.y = source[8] << 8 | source[9];
	pData->gyroscope.z = source[10] << 8 | source[11];
	pData->magnetometer.x = source[12] << 8 | source[13];
	pData->magnetometer.y = source[14] << 8 | source[15];
	pData->magnetometer.z = source[16] << 8 | source[17];
	pData->rotVectors.real = source[18] << 8 | source[19];
	pData->rotVectors.i = source[20] << 8 | source[21];
	pData->rotVectors.j = source[22] << 8 | source[23];
	pData->rotVectors.k = source[24] << 8 | source[25];

	// todo : is it necessary to scale them again ??
	pData->accelerometer.x *= scale;
	pData->accelerometer.y *= scale;
	pData->accelerometer.z *= scale;
	pData->gyroscope.x     *= scale;
	pData->gyroscope.y     *= scale;
	pData->gyroscope.z     *= scale;
	pData->rotVectors.real *= rotScale;
	pData->rotVectors.i    *= rotScale;
	pData->rotVectors.j    *= rotScale;
	pData->rotVectors.k    *= rotScale;

	return 26;

}

int rawToStruct_imu_6axis(unsigned int * source, unsigned int size, unsigned int n_elem, imu_data_t * pData, unsigned int scale)
{
	if(!source || !pData)
	{
		return 0;
	}

	pData->accelerometer.x = source[0] << 8 | source[1];
	pData->accelerometer.y = source[2] << 8 | source[3];
	pData->accelerometer.z = source[4] << 8 | source[5];
	pData->gyroscope.x = source[6] << 8 | source[7];
	pData-> gyroscope.y = source[8] << 8 | source[9];
	pData->gyroscope.z = source[10] << 8 | source[11];

	// todo : is it necessary to scale them again

	pData->accelerometer.x *= scale;
	pData->accelerometer.y *= scale;
	pData->accelerometer.z *= scale;
	pData->gyroscope.x     *= scale;
	pData->gyroscope.y     *= scale;
	pData->gyroscope.z     *= scale;

	return 12;
}

int rawToStruct_gpsMinData(unsigned char * source, unsigned int size, unsigned int n_elem, gps_data_t * pData)
{
	if (!source || !pData)
	{
		return 0;
	}

	pData->lon = bytesarrayToFloat(&source[0]);
	pData->lat = bytesarrayToFloat(&source[4]);
	pData->hMSL = bytesarrayToFloat(&source[8]);
	pData->gSpeed = bytesarrayToFloat(&source[12]);
	pData->gnssFixOK = source[16];


	return 17;
}

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD1(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d1 d1data;
	memset(&d1data, 0, sizeof(can_distance_node_d1));

	d1data.timestamp = HAL_GetTick();

	//todo: check size
	d1data.distance = source[0];

	*(can_distance_node_d1*) pData = d1data;

	return 1;
}

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD2(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d2 d2data;
	memset(&d2data, 0, sizeof(can_distance_node_d2));

	d2data.timestamp = HAL_GetTick();

	//todo: check size
	d2data.distance = source[0];

	*(can_distance_node_d2*) pData = d2data;

	return 1;
}



/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD3(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d3 d3data;
	memset(&d3data, 0, sizeof(can_distance_node_d3));

	d3data.timestamp = HAL_GetTick();

	//todo: check size
	d3data.calculatedDistance = (short) (source[0] << 8) | source[1];
	d3data.distanceUS = (short) (source[2] << 8) | source[3];
	d3data.distanceIR = source[4];

	*(can_distance_node_d3*) pData = d3data;

	return 1;
}

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD4(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d4 d4data;
	memset(&d4data, 0, sizeof(can_distance_node_d4));

	d4data.timestamp = HAL_GetTick();

	//todo: check size
	d4data.calculatedDistance = (short) (source[0] << 8) | source[1];
	d4data.distanceUS = (short) (source[2] << 8) | source[3];
	d4data.distanceIR1 = source[4];
	d4data.distanceIR2 = source[5];

	*(can_distance_node_d4*) pData = d4data;

	return 1;
}

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD5(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d5 d5data;
	memset(&d5data, 0, sizeof(can_distance_node_d5));

	d5data.timestamp = HAL_GetTick();

	//todo: check size
	d5data.calculatedDistance = (short) (source[0] << 8) | source[1];
	d5data.distanceUS = (short) (source[2] << 8) | source[3];
	d5data.distanceIR1 = source[4];
	d5data.distanceIR2 = source[5];
	d5data.distanceIR3 = source[6];

	*(can_distance_node_d5*) pData = d5data;

	return 1;
}

/* Converts incoming buffer of bytes to the appropriate structure */
int rawToStruct_canDistanceNodeD6(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d6 d6data;
	memset(&d6data, 0, sizeof(can_distance_node_d6));

	d6data.timestamp = HAL_GetTick();

	//todo: check size
	d6data.calculatedDistance = (short) (source[0] << 8) | source[1];
	d6data.distanceIR1 = source[2];
	d6data.distanceIR2 = source[3];
	d6data.distanceIR3 = source[4];
	d6data.distanceIR4 = source[5];

	*(can_distance_node_d6*) pData = d6data;

	return 1;
}

int rawToStruct_canDistanceNodeD7(unsigned char *source, unsigned int size, void * pData)
{
	/* Try the pointers */
	if (!source || !pData)
	{
		return 0;
	}
	can_distance_node_d6 d6data;
	memset(&d6data, 0, sizeof(can_distance_node_d6));

	d6data.timestamp = HAL_GetTick();

	//todo: check size
	d6data.calculatedDistance = (unsigned short) ((source[0] << 8) | source[1]);
	d6data.distanceIR1 = source[2];
	d6data.distanceIR2 = source[3];
	d6data.distanceIR3 = source[4];
	d6data.distanceIR4 = source[5];

	*(can_distance_node_d6*) pData = d6data;

	return 1;
}

int rawToStruct_canDistanceNodeD8(unsigned char * pSource, unsigned int size, void * pData)
{
	if(pSource == NULL)
		return 0;
	if(pData == NULL)
		return 0;

	can_distance_node_d5 d5data;

	d5data.timestamp = HAL_GetTick();

	//todo: check size
	d5data.calculatedDistance = (unsigned short) ((pSource[0] << 8) | pSource[1]);
	d5data.distanceUS = (unsigned short) ((pSource[2] << 8) | pSource[3]);
	d5data.distanceIR1 = pSource[4];
	d5data.distanceIR2 = pSource[5];
	d5data.distanceIR3 = pSource[6];

	*(can_distance_node_d5 *) pData = d5data;

	return 1;
}

