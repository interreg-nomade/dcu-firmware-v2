/*
 * freertos-i2c.h
 *
 *  Created on: May 14, 2018
 *      Author: alexis
 */

#ifndef FREERTOS_I2C_H_
#define FREERTOS_I2C_H_

#include <stdbool.h>
#include <stdint.h>

#include "stm32h7xx_hal.h"
#include "cmsis_os.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#include "sh2_hal_i2c.h"

//#include "i2c.h"


/* Setup the interface: uncomment the next line to use interrupts */
#define I2C_USING_IT

/* Setup the I2C interface used (1,2 or 3 for this version of microcontroller) */
#define I2C_PORT_USED 1

/* Enumeration of available I2C gates */
typedef enum
{
	I2C_0 = 0,
	I2C_1 = 1,
	I2C_2 = 2,
	I2C_3 = 3
}  I2C_PORT;


#ifdef I2C_PORT_USED
	#if I2C_PORT_USED == 1
		#define I2C_PORT_HANDLE hi2c1
	#elif I2C_PORT_USED == 2
		#define I2C_PORT_HANDLE hi2c2
	#elif I2C_PORT_USED == 3
		#define I2C_PORT_HANDLE hi2c3
	#else
		/* Wrong */
	#endif
#else
	/* Undefined */
#endif

extern I2C_HandleTypeDef I2C_PORT_HANDLE; /* The handle is initialized by CubeMX */
extern I2C_HandleTypeDef hi2c1;



/****************************************************************************************************************/
/* Basic function to interact with the I2C interface, these functions are protected using FreeRTOS capabilities */

/* Function to initialize semaphore and mutex used by the interface */
bool I2C_Init(); //todo: pass the port as argument

/* Write one byte */
bool I2C_WriteByte (I2C_PORT interface,				/* I2C interface to use */
						uint8_t device_address,		/* I2C's address of the device */
						uint8_t register_address,	/* Register address of the device to write */
						uint8_t* byte_to_write);	/* Pointer to a byte that contains the data to write */

/* Write several bytes */
bool I2C_WriteBytes(I2C_PORT interface,				/* I2C interface to use */
						uint8_t device_address,		/* I2C's address of the device */
						uint8_t register_address,	/* Register address of the device to write */
						uint8_t* data_byte,			/* Pointer to a byte array that contains the data to write */
						uint32_t num_bytes);		/* Number of bytes to write */

/* Read one byte */
bool I2C_ReadByte  (I2C_PORT interface,				/* I2C interface to use */
						uint8_t device_address,		/* I2C's address of the device */
						uint8_t register_address,	/* Register address to read */
						uint8_t* dest);				/* Pointer to a byte to store data */

/* Read several bytes */
bool I2C_ReadBytes (I2C_PORT interface,				/* I2C interface to use */
						uint8_t device_address,		/* I2C's address of the device */
						uint8_t register_address,	/* I2C's address of the device */
						uint8_t* data_byte,			/* Pointer to a byte array to store data */
						uint32_t num_bytes);		/* Number of bytes to read */
/****************************************************************************************************************/

#endif /* FREERTOS_I2C_H_ */
