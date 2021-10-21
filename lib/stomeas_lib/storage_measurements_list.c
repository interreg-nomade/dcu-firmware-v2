/**
 * @file storage_measurements_list.c
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date February 2020
 * @project Interreg EDUCAT
 */

#include "storage_measurements_list.h"
#include "ff.h"
#include "data_op/op.h"
#include "common.h"
#include <string.h>

#define MEASUREMENT_LIST_FILENAME "MEASLIST.BIN"

#define MEASLIST_SIZE_OF_WORKING_BUFFER 1024

static __attribute__((section(".sd_objects"))) FRESULT fr;
static __attribute__((section(".sd_objects"))) FILINFO fno;
static __attribute__((section(".sd_objects"))) uint8_t workingBuffer[MEASLIST_SIZE_OF_WORKING_BUFFER];
static __attribute__((section(".sd_objects"))) FIL filehandler;

#define WORKING_BUFFER_IS_BUSY 		0
#define WORKING_BUFFER_IS_AVAILABLE 1

#define MEASURE_TEST_MIN_TIME 1581527002
#define MEASURE_TEST_MAX_TIME 2212679002

uint32_t workingBufferStatus = 0;

static measurements_list_t * pmeaslist;

/* Private functions */
static measurement_list_op_result measurement_list_interpret_data(measurements_list_t* pmeas,
		uint8_t * buffer,
		uint32_t size);


measurement_list_op_result measurement_list_init(measurements_list_t * plist)
{
	if (!plist)
	{
		return meas_list_op_wrong_param;
	}
	else
	{
		pmeaslist = plist; /* Link the pointers */
	}

	measurement_list_op_result res;
	size_t size = sizeof(measurement_struct_t) * MEASUREMENT_LISTS_MAX_ELEMS;

	/* Put the link pointer in a known state */
	workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
	pmeaslist->companyID = 0;
	pmeaslist->elems = 0;
	pmeaslist->setupID = 0;
	pmeaslist->status = measurement_list_ressource_available;
	pmeaslist->userID = 0;
	pmeaslist->version = 0;
	pmeaslist->elems = 0;

	memset(pmeaslist->list, 0, size);

	/* Checks the existence of a MEASLIST.BIN file or sub-directory, fno handle the information about the object, size, timestamp and attribute */
	fr = f_stat(MEASUREMENT_LIST_FILENAME, &fno);


	if(fr == FR_NO_FILE)
	{
		return meas_list_op_err; /* The file doesn't exist */
	}
	else if(fr == FR_OK)
	{
		/* Measurement list file exist */
		if(fno.fsize)
		{
			/* Measuement list file contain data */
			return measurement_list_read_file();
		}
		else
		{
			return meas_list_op_err;
		}
	}

	return meas_list_op_err;
}

measurement_list_op_result measurement_list_read_file()
{
	FSIZE_t sizeOfFile = 0;
	measurement_list_op_result res;
	uint32_t freadcount = 0;

	/* Open the file */
	fr = f_open(&filehandler,
			MEASUREMENT_LIST_FILENAME,
			FA_OPEN_ALWAYS | FA_READ);

	if(fr != FR_OK)
	{
		/* ERROR WHEN OPPENIG THE MEASUREMENT LIST FILE */
		return meas_list_op_err;
	}

	/* Get the size of the file */
	sizeOfFile = f_size(&filehandler);
	/* Move to offset of 0 from top of the file */
	fr = f_lseek(&filehandler, 0);

	if (fr != FR_OK)
	{
		return meas_list_op_err;
	}

	if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
	{
		workingBufferStatus = WORKING_BUFFER_IS_BUSY;
		/* Check that the size is not too excessive */
		if ((sizeOfFile > 0) &&
				(sizeOfFile < (MEASLIST_SIZE_OF_WORKING_BUFFER-1)))
		{
			/* Read the entire file and put the data in the woekingBuffer */
			fr = f_read(&filehandler,
					workingBuffer,
					sizeOfFile,
					&freadcount);

			if(fr == FR_OK)
			{
				if (freadcount == sizeOfFile) /* Read successfull */
				{
					/* Interpret data */
					fr = measurement_list_interpret_data(pmeaslist,
							workingBuffer,
							freadcount);

					if (fr == meas_list_op_ok)
					{
						pmeaslist->status = measurement_list_correct; /* The measurement list struct is correct */
						workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE; // Make the buffer available
						f_close(&filehandler); //close the file as we don't need it
						return meas_list_op_ok;
					}
					else
					{
						workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE; // Make the buffer available
						f_close(&filehandler); //close the file as we don't need it
						return meas_list_op_ok;
					}
				}
				else
				{
					workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
					f_close(&filehandler);
					return meas_list_op_err;
				}
			}
			else
			{
				workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
				f_close(&filehandler);
				return meas_list_op_err;
			}
		}
		else
		{
			/* if the size of the file is too excessive for the working buffer do nothing */
			workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
			f_close(&filehandler);
			return meas_list_op_err;
		}
	}
	else
	{
		/* The working buffer is not available */
		f_close(&filehandler); // close the file
		return meas_list_op_err;
	}

	return meas_list_op_err;
}


static measurement_list_op_result measurement_list_interpret_data(measurements_list_t* pmeas,
		uint8_t * buffer,
		uint32_t size)
{
	uint8_t  byteSOH = buffer[0];
	uint16_t setupID  = (buffer[1] << 8) | buffer[2];
	uint16_t userID   = (buffer[3] << 8) | buffer[4];
	uint16_t version  = (buffer[5] << 8) | buffer[6];
	uint16_t companyID  = (buffer[7] << 8) | buffer[8];
	uint16_t numberOfMeasurements = (buffer[9] << 8) | buffer[10];

	uint64_t time = 0;

	uint32_t measurementID = 0;
	uint64_t start_time = 0;
	uint64_t stop_time = 0;
	uint32_t index = 0;

	//todo : for the future the number of measurement would be more
	if (numberOfMeasurements > MEASUREMENT_LISTS_MAX_ELEMS) // TODO : remove the old measuremment information from the measurement struct when its over
	{
		return meas_list_op_err;
	}

	uint8_t * pointer = &buffer[11]; /* Point at the first measurement ID */

	if(byteSOH == 0x01) /* CHECK if the first byte is equel to SOH */
	{
		pmeas->setupID = setupID;
		pmeas->userID = userID;
		pmeas->version = version;
		pmeas->companyID = companyID;
		pmeas->elems = 0;

		taskENTER_CRITICAL();
		app_rtc_get_unix_epoch_ms(&time);
		taskEXIT_CRITICAL();

		for (uint32_t i = 0; i < numberOfMeasurements; i++)
		{
			measurementID = pointer[0] << 24 | pointer[1] << 16 | pointer[2] << 8 | pointer[3];
			pointer += 4;

			bytes_array_to_uint64t_by_ref(pointer, &start_time);
			pointer  += 8;

			bytes_array_to_uint64t_by_ref(pointer, &stop_time);
			pointer  += 8;

			/* first check the coherence in the start_time and stop_time */
			if(start_time >= stop_time)
			{
				/* The start time can't be greater or equal to the the stop_time */
			}
			else if(time > stop_time)
			{
				/* the time is greater than the stop_time we are not
				 * in the range no utility to put it in the
				 * measurement list stuct
				 */
			}
			else if(time < start_time)
			{
				pmeaslist->list[index].id = measurementID;
				pmeaslist->list[index].startTime = start_time;
				pmeaslist->list[index].stopTime = stop_time;

				index++;
			}
			else if(start_time <= time && time < stop_time)
			{
				pmeaslist->list[index].id = measurementID;
				pmeaslist->list[index].startTime = start_time;
				pmeaslist->list[index].stopTime = stop_time;

				index++;
			}
		}

		if(index == 0)
		{
			/* No measurement possible */
			return meas_list_no_possibility; // todo : change to no no possibility
		}

		pmeaslist->elems = index;
		pmeaslist->status = measurement_list_correct;

		return meas_list_op_ok;
	}
	else
	{
		/* The data in the measurement list file is not consistent */
		return meas_list_op_err;
	}

	return meas_list_op_err;
}

unsigned int measurement_list_file_set_measurement_ID(uint32_t measurementID)
{
	uint32_t measurementIDFromFile;
	//uint8_t buffer[4] =


	/* We assume that the MEAS LIST file has been already open */

	flseek(&filehandler, 0); /* Put the read/write pointer at the begining of the file */

	flseek(&filehandler, f_tell(&filehandler) + 11); /* The offset come from the MEAS list struct the first measuremnet ID is in the index 11*/

	if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
	{
		workingBufferStatus = WORKING_BUFFER_IS_BUSY;

		memset(workingBuffer, 0, MEASLIST_SIZE_OF_WORKING_BUFFER);


		while(measurementIDFromFile != 0)
		{
			f_gets(workingBuffer,
					4,
					&filehandler);

			measurementIDFromFile = workingBuffer[0] << 24 | workingBuffer[1] << 16 | workingBuffer[2] << 8 | workingBuffer[3];
			flseek(&filehandler, f_tell(&filehandler) + 16); /* Go to the next measurement ID offset come from the stop_time and the start_time */
		}

		/* Build the working buffer to set the new measurement ID */
		workingBuffer[0] = measurementID >> 24;
		workingBuffer[1] = measurementID >> 16;
		workingBuffer[2] = measurementID >> 8;
		workingBuffer[3] = measurementID;
		/* Here we can change the measurement ID of the file */
		flseek(&filehandler, f_tell(&filehandler) - 4);

		fr = f_write(&filehandler,
				workingBuffer,
				4,
				NULL);
		if(fr == FR_OK)
		{
			return 1;
		}
	}
	else
	{
		return 0;
	}

	return 0;
}

measurement_list_op_result measurement_list_add(measurement_struct_t * pmeas)
{
	return meas_list_not_imp;
}

measurement_list_op_result measurement_list_remove(measurement_struct_t * pmeas)
{
	return meas_list_not_imp;
}

measurement_list_op_result measurement_list_destroy(measurement_struct_t * pmeas)
{
	return meas_list_not_imp;
}

unsigned int measurement_list_file_append(unsigned char * buffer,
		unsigned int size)
{
	uint32_t wBytes;
	/* Check the consistence of the input data */
	if(buffer == NULL || size == 0)
	{
		return 0;
	}

	/* Check if the MEAS LIST file exist */
	fr = f_stat(MEASUREMENT_LIST_FILENAME,
			&fno);

	switch(fr)
	{
	case FR_OK:

		if(fno.fsize == 0) /* The file is empty */
		{
			/* Open the file we can write in it */
			/* No file exist create a new file */
			fr = f_open(&filehandler, MEASUREMENT_LIST_FILENAME,
					FA_CREATE_ALWAYS | FA_WRITE);
			if(fr == FR_OK)
			{
				if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
				{
					workingBufferStatus = WORKING_BUFFER_IS_BUSY;

					memcpy(workingBuffer, buffer, size);

					fr = f_write(&filehandler,
							workingBuffer,
							size,
							(UINT *) &wBytes);

					workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;

					if(fr == FR_OK)
					{
						if(wBytes == size)
						{
							f_close(&filehandler);
							return 1;
						}
						else
						{
							return 0;
						}
					}
				}
			}
		}
		else /* The file is not empty */
		{
			/* For the moment delete the file and create a new one */
			fr = f_open(&filehandler, MEASUREMENT_LIST_FILENAME,
					FA_CREATE_ALWAYS | FA_WRITE);
			if(fr == FR_OK)
			{
				if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
				{
					workingBufferStatus = WORKING_BUFFER_IS_BUSY;

					memcpy(workingBuffer, buffer, size);

					fr = f_write(&filehandler,
							workingBuffer,
							size,
							(UINT *) &wBytes);

					workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;

					if(fr == FR_OK)
					{
						if(wBytes == size)
						{
							f_close(&filehandler);
							return 1;
						}
						else
						{
							return 0;
						}
					}
				}
			}

		}
		break;

	case FR_NO_FILE:
		/* No file exist create a new file */
		fr = f_open(&filehandler, MEASUREMENT_LIST_FILENAME,
				FA_CREATE_ALWAYS | FA_WRITE);
		if(fr == FR_OK)
		{
			if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
			{
				workingBufferStatus = WORKING_BUFFER_IS_BUSY;

				memcpy(workingBuffer, buffer, size);

				fr = f_write(&filehandler,
						workingBuffer,
						size,
						(UINT *) &wBytes);

				workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;

				if(fr == FR_OK)
				{
					if(wBytes == size)
					{
						f_close(&filehandler);
						return 1;
					}
					else
					{
						return 0;
					}
				}
			}
		}
		else
		{
			return 0;
		}
		break;

	default:
		break;
	}

	return 0;
}

//unsigned int measurement_list_file_append(unsigned char * buffer,
//		unsigned int size,
//		bool sync)
//{
//	unsigned int wBytes = 0;
//	unsigned int rBytes = 0;
//	uint16_t userIdFromTheBuffer = 0;
//	uint16_t userIdFromTheFile = 0;
//	uint16_t numberOfMeasurementFromTheFile = 0;
//	uint16_t numberOfMeasurementFromTheBuffer = 0;
//
//	if (buffer == NULL || size == 0)
//	{
//		return 0;
//	}
//
//	/* First check if a MEAS list file exist in the SD card */
//	fr = f_stat(MEASUREMENT_LIST_FILENAME, &fno);
//
//	switch(fr)
//	{
//	case FR_OK : /* File exist */
//	{
//		/* Delete the file */
//		f_unlink(MEASUREMENT_LIST_FILENAME);
//		/* First open the file */
//		fr = f_open(&filehandler,
//				MEASUREMENT_LIST_FILENAME,
//				FA_CREATE_NEW | FA_WRITE | FA_READ); //if the file doesn't exist the function f_open will create it
//
//		if(fr == FR_OK)
//		{
//			/* check if the file is empty or not */
//			if(f_size(&filehandler) == 0)
//			{
//				if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
//				{
//					/* Append data to the end of the file  */
//					f_lseek(&filehandler, 0);
//					/* The working buffer is available */
//					workingBufferStatus = WORKING_BUFFER_IS_BUSY; // Pass the status of the working buffer as busy
//					/* copy the contents of our buffer in the working buffer */
//					memset(workingBuffer, 0, size);
//					memcpy(workingBuffer, buffer, size);
//					/* Then we can write the contents of the working buffer in the MEAS list file */
//					fr = f_write (
//							&filehandler,      /* [IN] Pointer to the file object structure */
//							workingBuffer, 		/* [IN] Pointer to the data to be written */
//							size,         		/* [IN] Number of bytes to write */
//							&wBytes          		/* [OUT] Pointer to the variable to return number of bytes written */
//					);
//
//					workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
//
//					if(fr == FR_OK) /* check the result of the write operation */
//					{
//						if(size != wBytes)
//						{
//							/* The size is incomplete cause the number of bytes written is not equal to the size of the buffer */
//							/* Use of this function will handle the remains bytes to write in the file */
//							fr = f_sync(&filehandler);
//
//							if(fr == FR_OK)
//							{
//								f_close(&filehandler);
//								return 1;
//							}
//							else
//							{
//								/* Something went wrong during the sync operation
//								 * TODO : find a way to handle that case
//								 */
//								return 0;
//							}
//						}
//						else
//						{
//							f_close(&filehandler);
//							return 1;
//						}
//					}
//					else
//					{
//						/* Something went wrong during the write operation
//						 * TODO : find a way to handle that case
//						 */
//						return 0;
//					}
//				}
//				else
//				{
//					/* Working buffer is busy */
//					return 0;
//				}
//			}
//			else
//			{
//				/* The file is not empty in this case we need to check the USER ID
//				 * if the USER ID of the file and the buffer is not the same then
//				 * we clear the file and store a new one
//				 */
//				if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
//				{
//					workingBufferStatus = WORKING_BUFFER_IS_BUSY;
//
//					/* First we need to read the file */
//					fr = f_read (
//							&filehandler,     /* [IN] File object */
//							workingBuffer,  /* [OUT] Buffer to store read data */
//							fno.fsize,    /* [IN] Number of bytes to read */
//							&rBytes     /* [OUT] Number of bytes read */
//					);
//
//					if(fno.fsize == rBytes)
//					{
//						if(workingBuffer[0] == buffer[0]) /* The first byte of the measurement list file is SOH 0x01 */
//						{
//							userIdFromTheFile = (((workingBuffer[3] << 8) & 0xFF00) | (workingBuffer[4] & 0x00FF));
//							userIdFromTheBuffer = (((buffer[3] << 8) & 0xFF00) | (buffer[4] & 0x00FF));
//
//							if(userIdFromTheFile == userIdFromTheBuffer)
//							{
//								/* Keep the number of measurement already in the file */
//								numberOfMeasurementFromTheFile = ((workingBuffer[9] << 8) & 0xFF00) | (workingBuffer[10] & 0x00FF);
//								numberOfMeasurementFromTheBuffer = ((buffer[9] << 8) & 0xFF00) | (buffer[10] & 0x00FF);
//								/* if the user ID is the same we only add the new measurement to the existing file */
//								if(fno.fsize == (MEASLIST_SIZE_OF_WORKING_BUFFER - 1))
//								{
//									/* need to wrap the information in the measurement list file */
//									// todo : implement this feature wrap measurement list file information
//								}
//								else
//								{
//									/* Add the new measurement ID in continuity in the file */
//
//									/* Check if the read and write pointer are at the end of the file */
//									if(f_eof(&filehandler))
//									{
//										fpointer = f_tell(&filehandler);
//
//										f_lseek(&filehandler,
//												(fpointer - 2));
//
//										f_gets(workingBuffer,
//												2,
//												&filehandler);
//
//										if((workingBuffer[0] == 0x04) && (workingBuffer[1] == 0x03))
//										{
//											/* We are at the end of the measurement list file */
//											f_lseek(&filehandler,
//													(fpointer - 2));
//											/* Write know the new measurement ID information */
//											memset(workingBuffer, 0, (size - 11));  // the offset come from the header of the measurement list file
//											memcpy(workingBuffer, &buffer[11], (size - 11)); // the offset come from the header of the measurement list file
//
//											fr = f_write (
//													&filehandler,          /* [IN] Pointer to the file object structure */
//													&workingBuffer, /* [IN] Pointer to the data to be written */
//													(size - 11),         /* [IN] Number of bytes to write */
//													&wBytes          /* [OUT] Pointer to the variable to return number of bytes written */
//											);
//
//											if(fr == FR_OK)
//											{
//												if(wBytes == (size-11))
//												{
//													/* Then we need to update the number of measurement in the file */
//													numberOfMeasurementFromTheFile += numberOfMeasurementFromTheBuffer;
//													/* First we set the read write pointer at the Number of measurement index */
//													f_lseek(&filehandler, 9);
//
//													workingBuffer[0] = ((numberOfMeasurementFromTheFile << 8) & 0x00FF);
//													workingBuffer[0] = (numberOfMeasurementFromTheFile & 0x00FF);
//
//													fr = f_write (
//															&filehandler,          /* [IN] Pointer to the file object structure */
//															&workingBuffer, /* [IN] Pointer to the data to be written */
//															2,         /* [IN] Number of bytes to write */
//															&wBytes          /* [OUT] Pointer to the variable to return number of bytes written */
//													);
//
//													workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
//
//													if(fr == FR_OK)
//													{
//														if(wBytes == 2)
//														{
//															/*
//															 * TODO : maybe close the file would be a good idea
//															 */
//															return 1;
//														}
//													}
//													else
//													{
//														/*
//														 * Error
//														 */
//														return 0;
//													}
//												}
//											}
//										}
//										else
//										{
//											/* The end of the measurement list file is not as specified
//											 * EOT and ETX are missing...
//											 */
//											return 0;
//										}
//									}
//									else
//									{
//										fpointer = f_size(&filehandler); // get the size of the file
//
//										f_lseek(&filehandler, fpointer); // go at the end of the file
//
//										//TODO : BEFORE TEST AND THEN IMPLEMENT THIS PART see line 543 of this file
//									}
//								}
//							}
//							else
//							{
//								/* if it not the same user ID then we remove the file to create a new one */
//								f_close(&filehandler);
//								fr = f_unlink (MEASUREMENT_LIST_FILENAME);  /* [IN] Object name */
//
//								if(fr == FR_OK)
//								{
//									/* Create a new file */
//									fr = f_open(&filehandler, MEASUREMENT_LIST_FILENAME, FA_OPEN_APPEND | FA_WRITE | FA_READ); //if the file doesn't exist the function f_open will create it
//									if(fr == FR_OK)
//									{
//										/* copy the contents of our buffer in the working buffer */
//										memset(workingBuffer, 0, size);
//										memcpy(workingBuffer, buffer, size);
//										/* Then we can write the contents of the working buffer in the MEAS list file */
//										fr = f_write (
//												&filehandler,      /* [IN] Pointer to the file object structure */
//												workingBuffer, 		/* [IN] Pointer to the data to be written */
//												size,         		/* [IN] Number of bytes to write */
//												&wBytes          		/* [OUT] Pointer to the variable to return number of bytes written */
//										);
//
//										workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
//
//										if(fr == FR_OK) /* check the result of the write operation */
//										{
//											if(size != wBytes)
//											{
//												/* The size is incomplete cause the number of bytes written is not equal to the size of the buffer */
//												/* Use of this function will handle the remains bytes to write in the file */
//												fr = f_sync(&filehandler);
//
//												if(fr == FR_OK) return 1;
//
//												else
//												{
//													/* Something went wrong during the write operation
//													 * TODO : find a way to handle that case
//													 */
//													return 0;
//												}
//											}
//											else
//											{
//												return 1;
//											}
//										}
//										else
//										{
//											/* Something went wrong during the write operation
//											 * TODO : find a way to handle that case
//											 */
//											return 0;
//										}
//									}
//								}
//								else
//								{
//									/* Something went wrong during deleting the file */
//									return 0;
//								}
//							}
//						}
//						else
//						{
//							/* ERROR the first byte is not the SOH has expected
//							 * TODO : find a way to handle that error */
//							return 0;
//						}
//					}
//					else
//					{
//						/* The file has not been read completely
//						 * TODO : find a way to handle that case
//						 */
//						return 0;
//					}
//				}
//				else
//				{
//					/* The working buffer is busy */
//					return 0;
//				}
//			}
//		}
//		else
//		{
//			/* Something went wrong during the open file operation
//			 * todo : find a way to handle that case */
//			return 0;
//		}
//
//		break;
//	}
//	case FR_NO_FILE : /* File doesn't exist */
//	{
//		/* First create the file */
//		fr = f_open(&filehandler, MEASUREMENT_LIST_FILENAME,
//				FA_CREATE_ALWAYS | FA_WRITE); //if the file doesn't exist the function f_open will create it
//
//		if(fr == FR_OK)
//		{
//			/* At this point no file exist and the file is empty */
//
//			/* We can now store the data send by the tablet to the file */
//			/* We need to the copy the buffer in the working buffer cause
//			 * the working buffer is placed in the RAM 1 and it is our gateway
//			 * to the SD card
//			 */
//
//			if(workingBufferStatus == WORKING_BUFFER_IS_AVAILABLE)
//			{
//				f_lseek(&filehandler, 0); // move the read/write pointer at the begining of the file
//				/* The working buffer is available */
//				workingBufferStatus = WORKING_BUFFER_IS_BUSY; // Pass the status of the working buffer as busy
//				/* copy the contents of our buffer in the working buffer */
//				memset(workingBuffer, 0, size);
//				memcpy(workingBuffer, buffer, size);
//				/* Then we can write the contents of the working buffer in the MEAS list file */
//				fr = f_write (
//						&filehandler,      /* [IN] Pointer to the file object structure */
//						workingBuffer, 		/* [IN] Pointer to the data to be written */
//						size,         		/* [IN] Number of bytes to write */
//						&wBytes          		/* [OUT] Pointer to the variable to return number of bytes written */
//				);
//
//				workingBufferStatus = WORKING_BUFFER_IS_AVAILABLE;
//
//				if(fr == FR_OK) /* check the result of the write operation */
//				{
//					if(size != wBytes)
//					{
//						/* The size is incomplete cause the number of bytes written is not equal to the size of the buffer */
//						/* Use of this function will handle the remains bytes to write in the file */
//						fr = f_sync(&filehandler);
//
//						if(fr == FR_OK)
//						{
//							f_close(&filehandler);
//							return 1;
//						}
//						else
//						{
//							/* Something went wrong during the write operation
//							 * TODO : find a way to handle that case
//							 */
//							return 0;
//						}
//					}
//					else
//					{
//						f_close(&filehandler);
//						return 1;
//					}
//				}
//				else
//				{
//					/* Something went wrong during the write operation
//					 * TODO : find a way to handle that case
//					 */
//					return 0;
//				}
//			}
//			else
//			{
//				/* The working buffer is not available
//				 *TODO : find a way to handle that case */
//				return 0;
//			}
//
//		}
//		else
//		{
//			/* Something went wrong during the open file operation
//			 * todo : find a way to handle that case */
//			return 0;
//		}
//		break;
//	}
//	default: /* An error occur on the file system or in the path */
//	{
//		return 0;
//		break;
//	}
//	}
//
//	return 1;
//}
