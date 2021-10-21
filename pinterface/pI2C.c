/*
 * freertos-i2c.c
 *
 *  Created on: May 14, 2018
 *      Author: alexis
 */

#include "pI2C.h"
#include <stdbool.h>

static SemaphoreHandle_t i2cMutex;
static SemaphoreHandle_t i2cBlockSem;

/* Function to initialize semaphore and semaphore used by the interface */
bool I2C_Init()
{
	/* Create a semaphore for mutual exclusion between task */
	i2cMutex = xSemaphoreCreateMutex();
	/* Create semaphore for synchronization between task or between interrupt and task */
	i2cBlockSem = xSemaphoreCreateBinary();

	if(i2cMutex != NULL)
	{
		/* The semaphore was created successfully and can be used */
	}
	else
	{
		return 0;
	}

	if(i2cBlockSem != NULL)
	{
		/* The binary semaphore was created successfully */
		/* The semaphore must be given first time in order to be take */
		return 1;
	}
	else
	{
		return 0;
	}
	return 0;
}


bool I2C_WriteByte (I2C_PORT interface,
		uint8_t device_address,
		uint8_t register_address,
		uint8_t * byte_to_write)
{
#ifdef I2C_USING_IT
	HAL_StatusTypeDef res;

	/* Need to take the mutex first as other task will use
	 * this ressource
	 */
	if(xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
	{
		/* I2C operation */
		res = HAL_I2C_Mem_Write_IT(&I2C_PORT_HANDLE,
				device_address,
				register_address,
				1,
				byte_to_write,
				1);

		if(res == HAL_OK)
		{
			/* I2C operation succeed */
			/* Block on the semaphore
			 * the semaphore is release in the I2C interrupt callback
			 */
			if(xSemaphoreTake(i2cBlockSem, (TickType_t) 1000) == pdTRUE)
			{
				/* Release the mutex */
				xSemaphoreGive(i2cMutex);
				return true;
			}
		}

		/* Release the mutex */
		xSemaphoreGive(i2cMutex);
	}

	return false;

	//	if (xSemaphoreTake(i2cBlockSem, portMAX_DELAY) == pdTRUE)
	//	{
	//		/* Take Semaphore success */
	//		/* I2C operation */
	//		res = HAL_I2C_Mem_Write_IT(&I2C_PORT_HANDLE,
	//				device_address,
	//				register_address,
	//				1,
	//				byte_to_write,
	//				1);
	//
	//		if(res == HAL_OK)
	//		{
	//			/* I2C operation succeed */
	//			/* Release the semaphore is made in the interrupt callback function */
	//			return true;
	//		}
	//		else
	//		{
	//			/* I2C operation failed */
	//			/* Release the semaphore */
	//			xSemaphoreGive(i2cBlockSem);
	//			return false;
	//		}
	//
	//	}
	//	else
	//	{
	//		/* Timeout take the semaphore failed */
	//		return false;
	//	}
	//
	//	return false;

#else
	return false;
#endif
}


/* Write several bytes */
bool I2C_WriteBytes(I2C_PORT interface,
		uint8_t device_address,
		uint8_t register_address,
		uint8_t* data_byte,
		uint32_t num_bytes)
{
#ifdef I2C_USING_IT
	HAL_StatusTypeDef res;
	if(xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
	{
		/* I2C operation */
		res = HAL_I2C_Mem_Write_IT(&I2C_PORT_HANDLE,
				device_address,
				register_address,
				1,
				data_byte,
				num_bytes);
		if(res == HAL_OK)
		{
			/* I2C operation succeed */
			/* Block on the semaphore
			 * the semaphore is release in the I2C interrupt callback
			 */
			if(xSemaphoreTake(i2cBlockSem, (TickType_t) 1000) == pdTRUE)
			{
				/* Release the mutex */
				xSemaphoreGive(i2cMutex);
				return true;
			}
		}

		/* Release the mutex */
		xSemaphoreGive(i2cMutex);
	}

	return false;

	//	if (xSemaphoreTake(i2cBlockSem, portMAX_DELAY) == pdTRUE)
	//	{
	//		//take success
	//		res = HAL_I2C_Mem_Write_IT(&I2C_PORT_HANDLE,
	//				device_address,
	//				register_address,
	//				1,
	//				data_byte,
	//				num_bytes);
	//
	//		if(res == HAL_OK)
	//		{
	//			/* I2C operation success */
	//			/* Release of the semaphore is made in the interrupt callback function */
	//			/* Success */
	//			return true;
	//		}
	//		else
	//		{
	//			/* I2C operation failed */
	//			/* Release the semaphore */
	//			xSemaphoreGive(i2cBlockSem);
	//			/* Error */
	//			return false;
	//		}
	//	}
	//	else
	//	{
	//		/* Timeout failed to take the semaphore */
	//		return false;
	//	}
	//
	//	return false;

#else
	return false;
#endif
}

/* Read one byte */
bool I2C_ReadByte  (I2C_PORT interface,			    /* I2C interface to use */
		uint8_t device_address,		/* I2C's adress of the device */
		uint8_t register_address,	/* Register adress to read */
		uint8_t* dest)				/* Pointer to a byte to store data */
{
#ifdef I2C_USING_IT
	HAL_StatusTypeDef res;

	if(xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
	{
		/* I2C operation */
		res = HAL_I2C_Mem_Read_IT(&I2C_PORT_HANDLE,
				device_address,
				register_address,
				1,
				dest,
				1);

		if(res == HAL_OK)
		{
			/* I2C operation succeed */
			/* Block on the semaphore
			 * the semaphore is release in the I2C interrupt callback
			 */
			if(xSemaphoreTake(i2cBlockSem, (TickType_t) 1000) == pdTRUE)
			{
				/* Release the mutex */
				xSemaphoreGive(i2cMutex);
				return true;
			}
		}

		/* Release the mutex */
		xSemaphoreGive(i2cMutex);
	}

	return false;

	//	HAL_StatusTypeDef res;
	//	if (xSemaphoreTake(i2cBlockSem, portMAX_DELAY) == pdTRUE)
	//	{
	//		/* Take the semaphore success */
	//		/* I2C operation */
	//		res = HAL_I2C_Mem_Read_IT(&I2C_PORT_HANDLE,
	//				device_address,
	//				register_address,
	//				1,
	//				dest,
	//				1);
	//
	//		if(res == HAL_OK)
	//		{
	//			/* I2C operation success */
	//			/* Release of the semaphore is made in the interrupt callback function */
	//			return true;
	//		}
	//		else
	//		{
	//			/* I2C operation failed */
	//			/* Release the semaphore */
	//			xSemaphoreGive(i2cBlockSem);
	//			return false;
	//		}
	//	}
	//
	//	return false;
#else
	return false;
#endif

}

/* Read several bytes */
bool I2C_ReadBytes (I2C_PORT interface,			    /* I2C interface to use */
		uint8_t device_address,						/* I2C's adress of the device */
		uint8_t register_address,					/* I2C's adress of the device */
		uint8_t * data_byte,						/* Pointer to a byte array to store data */
		uint32_t num_bytes)							/* Number of bytes to read */
{
#ifdef I2C_USING_IT
	HAL_StatusTypeDef res;

	if(xSemaphoreTake(i2cMutex, portMAX_DELAY) == pdTRUE)
	{
		/* I2C operation */
		res = HAL_I2C_Mem_Read_IT(&I2C_PORT_HANDLE,
				device_address,
				register_address, 1,
				data_byte,
				num_bytes);

		if(res == HAL_OK)
		{
			/* I2C operation succeed */
			/* Block on the semaphore
			 * the semaphore is release in the I2C interrupt callback
			 */
			if(xSemaphoreTake(i2cBlockSem, (TickType_t) 1000) == pdTRUE)
			{
				/* Release the mutex */
				xSemaphoreGive(i2cMutex);
				return true;
			}
		}

		/* Release the mutex */
		xSemaphoreGive(i2cMutex);
	}

	return false;
	//
	//	if (xSemaphoreTake(i2cBlockSem, portMAX_DELAY) == pdTRUE)
	//	{
	//		/* Take semaphore success */
	//		/* I2C operation */
	//		res = HAL_I2C_Mem_Read_IT(&I2C_PORT_HANDLE,
	//				device_address,
	//				register_address, 1,
	//				data_byte,
	//				num_bytes);
	//
	//		if(res == HAL_OK)
	//		{
	//			/* I2C operation success */
	//			/* Release of the semaphore is made in the interrupt callback function */
	//			/* Success */
	//			return true;
	//		}
	//		else
	//		{
	//			/* I2C operation failed */
	//			/* Release the semaphore */
	//			xSemaphoreGive(i2cBlockSem);
	//			/* Error during the I2C operation */
	//			return false;
	//		}
	//	}
	//	else
	//	{
	//		/* Timeout unable to take the semaphore */
	//		return false;
	//	}
	//	return false;
#else
	//TODO : Implement in polling mode using mutex
	return false;
#endif
}

/* Callback functions */

/* MemTx complete callback */

#ifdef I2C_USING_IT
void HAL_I2C_MemTxCpltCallback(I2C_HandleTypeDef * hi2c)
{
	if(hi2c == &I2C_PORT_HANDLE)
	{
		static BaseType_t woken;

		woken = pdFALSE;

		xSemaphoreGiveFromISR(i2cBlockSem, &woken);

		portYIELD_FROM_ISR(woken);
	}
//	else if(hi2c == &hi2c4)
//	{
//
//	}
}


void HAL_I2C_MemRxCpltCallback(I2C_HandleTypeDef *hi2c)
{
	if(hi2c == &I2C_PORT_HANDLE)
	{
		static BaseType_t woken;

		woken = pdFALSE;

		xSemaphoreGiveFromISR(i2cBlockSem, &woken);

		portYIELD_FROM_ISR(woken);
	}
//	else if(hi2c == &hi2c4)
//	{
//
//	}
}

void HAL_I2C_ErrorCallback(I2C_HandleTypeDef * hi2c)
{
	if(hi2c == &I2C_PORT_HANDLE)
	{
		static BaseType_t woken;

		woken = pdFALSE;

		xSemaphoreGiveFromISR(i2cBlockSem, &woken);

		portYIELD_FROM_ISR(woken);
	}
//	else if(hi2c == &hi2c4)
//	{
//		BNO_I2C_ErrorCallback(hi2c);
//	}
}


void HAL_I2C_MasterTxCpltCallback(I2C_HandleTypeDef * hi2c)
{
//	if (hi2c == &hi2c4)
//	{
//		BNO_I2C_MasterTxCpltCallback(hi2c);
//	}
}

void HAL_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * hi2c)
{
//	if (hi2c == &hi2c4)
//	{
//		BNO_I2C_MasterRxCpltCallback(hi2c);
//	}
}


#endif
