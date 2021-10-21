#include "storage_measurements.h"
#include "data_op/op.h"
#include <string.h>

static RAM_PLACEMENT FRESULT fr;
static RAM_PLACEMENT FILINFO fno;
static RAM_PLACEMENT uint8_t workingBuffer[1024];
/**
 * @fn stomeas_res_t storage_meas_create_file( stomeas_handler_t * pHandler, char * array, uint32_t array_len )
 * @brief Create a new measurement file
 * @param pHandler Pointer to the storage measurement handler
 * @param array Array containing the name of the file
 * @param array_len Length of the array containing the name of the file
 * @param clear
 * @return Result of the operation
 */
stomeas_res_t storage_meas_create_file(stomeas_handler_t * pHandler,
		char * nameOfFile,
		uint8_t clear)
{
	if (pHandler == NULL)
		return STOMEAS_OP_WRONG_PARAM;
	if (nameOfFile == NULL)
		return STOMEAS_OP_WRONG_PARAM;

	fr = f_stat(nameOfFile, &fno);

	if (fr == FR_OK)
	{
		/* File exists */
		if (clear)
		{
			f_unlink(nameOfFile);
		}
		else
		{
			return STOMEAS_OP_FILE_EXISTS_WRONG_OP;
		}
	}

	fr = f_open(&pHandler->file_handler,
			nameOfFile,
			FA_CREATE_NEW | FA_WRITE | FA_READ);

	if (fr == FR_OK)
	{
		f_lseek (&pHandler->file_handler, STOMEAS_HEADER_SIZE);
		return STOMEAS_OP_OK;
	}
	else
	{
		return STOMEAS_OP_ERROR;
	}
}


/**
 * @fn stomeas_res_t storage_meas_check_file_exists( stomeas_handler_t * pHandler, char * array, uint32_t array_len )
 * @brief Look if a specific file exists
 * @param array Array containing the name of the file
 * @return Result of the operation
 */
stomeas_res_t storage_meas_check_file_exists(char * array)
{
	fr = f_stat(array, &fno);

	if (fr == FR_OK)
	{
		return STOMEAS_OP_FILE_EXISTS;
	}
	else if (fr == FR_NO_FILE)
	{
		return STOMEAS_OP_FILE_DOES_NOT_EXIST;
	}
	return STOMEAS_OP_ERROR;
}


/**
 * @fn stomeas_res_t storage_meas_set_file( stomeas_handler_t * pHandler, char * array, uint32_t array_len )
 * @brief Sets handler to work on the specified file
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param array Array containing the name of the file
 * @param array_len Length of the array containing the name of the file
 * @return Result of the operation
 */
stomeas_res_t storage_meas_set_file(stomeas_handler_t * pHandler, char * array, uint32_t array_len )
{
	if (pHandler == NULL)
		return STOMEAS_OP_WRONG_PARAM;
	if (array == NULL)
		return STOMEAS_OP_WRONG_PARAM;

	pHandler->valid = false;

	fr = f_open(&pHandler->file_handler, array, FA_OPEN_APPEND | FA_WRITE | FA_READ);

	if (fr == FR_OK)
	{
		/* Move the read and write pointer of the file
		 * to let space for the header part */
		f_lseek(&pHandler->file_handler, STOMEAS_HEADER_SIZE);
		pHandler->valid = true;
		return STOMEAS_OP_OK;
	}
	else
	{
		return STOMEAS_OP_ERROR;
	}
}


/**
 * @fn stomeas_res_t storage_meas_get_file_infos( FILINFO * pInfos, char * array, uint32_t array_len )
 * @brief Get infos on a specific file
 *
 * @param pInfos Pointer to the FILFINO struct which will contain results of the operation
 * @param array Array containing the name of the file
 * @param array_len Length of the array containing the name of the file
 * @return Result of the operation
 */
stomeas_res_t storage_meas_get_file_infos( FILINFO * pInfos, char * array, uint32_t array_len )
{
	//let's pretend the string is always correctly parsed
	fr = f_stat(array, pInfos);

	if (fr == FR_OK)
	{
		return STOMEAS_OP_FILE_EXISTS;
	}
	else if (fr == FR_NO_FILE)
	{
		return STOMEAS_OP_FILE_DOES_NOT_EXIST;
	}
	else
	{
		return STOMEAS_OP_ERROR;
	}
}


/**
 * @fn stomeas_res_t storage_meas_get_header( stomeas_handler_t * pHandler);
 * @brief Get header of the file previously opened through storage_meas_set_file.
 * 	      Header in the handler will be updated if operation succedded
 *
 * @param pHandler Pointer to the storage measurement handler
 * @return Result of the operation
 */
stomeas_res_t storage_meas_get_header( stomeas_handler_t * pHandler, stomeas_header_t * pHeader)
{
	uint32_t freadcount = 0;
	/* Move to offset of 0 from top of the file */
	fr = f_lseek(&pHandler->file_handler, 0);

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}
	workingBuffer[0] = 0;
	memset(workingBuffer, 0, STOMEAS_HEADER_SIZE);

	fr = f_read(&pHandler->file_handler,
			workingBuffer,
			STOMEAS_HEADER_SIZE,
			&freadcount);  /* Read a chunk of source file */
	//TODO : check if bytes read equal to the size of the header

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}

	/* Now data was correctly retrieved from index 0 of the file. */
	if (workingBuffer[0] != STOMEAS_START_OF_HEADING)
	{
		return STOMEAS_OP_ERROR;
	}
	if (workingBuffer[STOMEAS_HEADER_SIZE-1] != STOMEAS_START_OF_TEXT)
	{
		return STOMEAS_OP_ERROR;
	}
	/* Otherwise, the delimiters are correct */

	pHeader->setup_id = workingBuffer[1] << 8 | workingBuffer[2];
	pHeader->version  = workingBuffer[3] << 8 | workingBuffer[4];
	pHeader->company_id = workingBuffer[5] << 8 | workingBuffer[6];
	pHeader->measurement_id = buffer_to_unsigned_int(&workingBuffer[7]);
	pHeader->start_time = bytes_array_to_uint64t(&workingBuffer[11]);
	pHeader->stop_time = bytes_array_to_uint64t(&workingBuffer[19]);
	pHeader->block_length = workingBuffer[27] << 8 | workingBuffer[28];

	pHeader->valid = 1; /* TODO: CReate a function to check validity */

	return STOMEAS_OP_OK;
}

/**
 * @fn stomeas_res_t storage_meas_set_header( stomeas_handler_t * pHandler);
 * @brief Set header of a file, will also update the header contained in the handler
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to the header to write
 * @return Result of the operation
 */
stomeas_res_t storage_meas_set_header( stomeas_handler_t * pHandler, stomeas_header_t * pHeader)
{
	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;

	uint8_t *p = NULL;
	uint32_t bw = 0;

	workingBuffer[0] = STOMEAS_START_OF_HEADING;
	workingBuffer[1] = pHeader->setup_id << 8;
	workingBuffer[2] = pHeader->setup_id;

	workingBuffer[3] = pHeader->version << 8;
	workingBuffer[4] = pHeader->version;

	workingBuffer[5] = pHeader->company_id << 8;
	workingBuffer[6] = pHeader->company_id;

	p = &workingBuffer[7];

	unsigned_int_to_byte_array(pHeader->measurement_id, p);


	p = &workingBuffer[11];

	uint64t_to_bytes_array(pHeader->start_time, p);

	p = &workingBuffer[19];

	uint64t_to_bytes_array(pHeader->stop_time, p);

	workingBuffer[27] = pHeader->block_length >> 8;
	workingBuffer[28] = pHeader->block_length;
	workingBuffer[29] = STOMEAS_START_OF_TEXT;

	/* Before move the read and write pointer flush the file
	 * buffer by using f_sync function */
	f_sync(&pHandler->file_handler);

	/* Set the pointer to the begin of the file */
	fr = f_lseek(&pHandler->file_handler, 0);

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}

	/* Start writing */
	fr = f_write(&pHandler->file_handler,
			workingBuffer,
			STOMEAS_HEADER_SIZE,
			(UINT *)&bw);

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}
	else if (fr == FR_OK)
	{
		fr = f_sync ( &pHandler->file_handler ); /* Flush */
		return STOMEAS_OP_OK;
	}
	else
	{
		/* Unhandled error ! */
	}
	return STOMEAS_OP_ERROR;
}



/**
 * @fn stomeas_res_t storage_meas_search_block_from_timestamp( stomeas_handler_t * pHandler, uint64_t timestamp)
 * @brief Search for a block
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param timestamp Timestamp to look for
 * @return Result of the operation
 */
stomeas_res_t storage_meas_search_block_from_timestamp( stomeas_handler_t * pHandler,  stomeas_header_t *pHeader, uint64_t timestamp)
{
	// NOT IMPLEMENTED YET
	uint32_t freadcount = 0;

	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;

#if 0
	pHeader->start_time

	/* Set the pointer to the end of the header */
	fr = f_lseek(&pHandler->file_handler, STOMEAS_HEADER_SIZE);

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}
	fr = f_read(&pHandler->file_handler, workingBuffer, STOMEAS_HEADER_SIZE, &freadcount);  /* Read a chunk of source file */

	if (fr != FR_OK)
	{
		return STOMEAS_OP_ERROR;
	}
#endif
	return STOMEAS_OP_ERROR; // NOT IMPLEMENTED YET
}

/**
 * @fn stomeas_res_t storage_meas_get_block( stomeas_handler_t * pHandler, uint64_t block_number, uint8_t * pDest)
 * @brief Get content of a block.
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param block_number number of the block
 * @param pDest pointer to the destination array
 * @return Result of the operation
 */
stomeas_res_t storage_meas_get_block( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint64_t block_number, uint8_t * pDest)
{
	return 1;
}

/**
 * @fn stomeas_res_t storage_meas_util_verify_integrity_size( stomeas_handler_t * pHandler, stomeas_header_t *pHeader)
 * @brief Check through the size of the file if nothing is missing (e.g. board shutdown while writing on sd --> only half a block written).
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @return Result of the operation
 */
stomeas_res_t storage_meas_util_verify_integrity_size( stomeas_handler_t * pHandler, stomeas_header_t *pHeader)
{
	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;

	uint32_t sizeOfFile = f_size(&pHandler->file_handler);
	uint32_t sizeOfBlocks = sizeOfFile - STOMEAS_HEADER_SIZE;
	if ( (sizeOfBlocks%pHeader->block_length) == 0)
	{
		//Everything is fine
		return STOMEAS_OP_OK;
	}
	return STOMEAS_OP_FILE_CORRUPT;
}


/**
 * @fn stomeas_res_t storage_meas_get_block( stomeas_handler_t * pHandler, uint64_t block_number, uint8_t * pDest)
 * @brief Append a char array to the measurement file.
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param block_number number of the block
 * @param pDest pointer to the destination array
 * @return Result of the operation
 */
stomeas_res_t storage_meas_append_block( stomeas_handler_t * pHandler, stomeas_header_t *pHeader)
{
	uint32_t bw = 0;

	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;

	fr = f_lseek(&pHandler->file_handler, f_size(&pHandler->file_handler));
	if (fr == FR_OK)
	{
		if (!pHandler->bufferUsed)
		{
			pHandler->bufferUsed = true;
			fr = f_write(&pHandler->file_handler, pHandler->buffer, pHeader->block_length, (unsigned int *) &bw);
			if (fr == FR_OK)
			{
				fr = f_sync (
						&pHandler->file_handler     /* [IN] File object */
				);
				if (fr == FR_OK)
				{
					pHandler->bufferUsed = false;
				}
				return STOMEAS_OP_OK;
			}
			else
			{
				return STOMEAS_OP_ERROR;
			}
		}
		else
		{
			return STOMEAS_OP_ERROR_BUFFER_USED;
		}
	}
	else
	{
		/* Error to handle */
	}
	return STOMEAS_OP_ERROR;

}

/**
 * @fn stomeas_res_t storage_meas_convert_timestamp_to_block_number( uint32_t blockSize, uint64_t timestamp, uint32_t * pBlockNumber);
 * @brief Convert a timestamp to a block number, depending on the header of a measurement's file
 *
 * @param blockSize Pointer to a header handler - need to know block size
 * @param timestamp specified timestamp we're looking for
 * @param pBlockNumber Pointer to the result
 * @return Result of the operation
 */
stomeas_res_t storage_meas_convert_timestamp_to_block_number( uint32_t blockSize, uint64_t timestamp, uint32_t * pBlockNumber);


/**
 * @fn stomeas_res_t storage_meas_convert_data_to_block( stomeas_handler_t * pHandler, decoded_config_t *pConfig, uint8_t * pDest, uint32_t * pLength)
 * @brief From a configuration input, reads data of each instrument, and convert it to a block inside a char array.
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param blockLength Length of a block
 * @param pConfig Pointer to the configuraiton
 * @param pDest Pointer to the destination buffer
 * @param pLength Pointer to the variable handling length result
 * @param pDest pointer to the destination array
 * @return Result of the operation
 */
stomeas_res_t storage_meas_convert_data_to_block( stomeas_handler_t * pHandler, uint32_t blockLength, decoded_config_t *pConfig, uint8_t * pDest, uint32_t * pLength);


/**
 * @fn stomeas_res_t storage_meas_util_fill_zeros( stomeas_handler_t * pHandler, uint32_t nElems, stomeas_header_t *pHeader)
 * @brief Append a char array to the measurement file.
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param nElems number of elements written in buffer
 * @param pDest pointer to the destination array
 * @return Result of the operation
 */
stomeas_res_t storage_meas_util_fill_zeros( stomeas_handler_t * pHandler, stomeas_header_t *pHeader)
{

	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;
	if (pHeader->block_length > STOMEAS_HANDLER_BUFFER_SIZE)
		return STOMEAS_OP_WRONG_PARAM; /* Block length is larger than the buffer available */

	if (!pHandler->bufferUsed)
	{
		pHandler->bufferUsed = true;
		if (!pHandler->buffer_elems)
		{
			pHandler->bufferUsed = false;
			/* Zero elements: block's header is not even built! */
			return STOMEAS_OP_WRONG_PARAM;
		}
		else
		{
			uint32_t missingBytes = pHeader->block_length - pHandler->buffer_elems; // Number of bytes missing in the block
			uint8_t * startingPointer = pHandler->buffer + pHandler->buffer_elems;  // Starting pointer
			//printf("missing bytes: %d\n", missingBytes);
			memset(startingPointer+1, 0, missingBytes );
			pHandler->buffer_elems = pHeader->block_length;
			pHandler->bufferUsed = false;
			return STOMEAS_OP_OK;
		}
	}
	return STOMEAS_OP_ERROR;
}

stomeas_res_t storage_meas_util_build_block_header( stomeas_handler_t * pHandler,
		stomeas_header_t *pHeader,
		uint64_t epochms,
		uint16_t measurementStatus)
{

	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;
	if (!pHeader)
		return STOMEAS_OP_WRONG_PARAM;
	if (pHeader->block_length > STOMEAS_HANDLER_BUFFER_SIZE)
		return STOMEAS_OP_WRONG_PARAM; /* Block length is larger than the buffer available */

	if (!pHandler->bufferUsed)
	{
		uint32_t timestamp_offset = epochms - (pHeader->start_time);
		pHandler->bufferUsed = true;
		pHandler->buffer[0] = timestamp_offset >> 24 & 0xff;
		pHandler->buffer[1] = timestamp_offset >> 16 & 0xff;
		pHandler->buffer[2] = timestamp_offset >> 8  & 0xff;
		pHandler->buffer[3] = timestamp_offset       & 0xff;
		pHandler->buffer[4] = measurementStatus >> 8 & 0xff;
		pHandler->buffer[5] = measurementStatus      & 0xff;
		pHandler->buffer_elems = 6;
		pHandler->bufferUsed = false;
		return STOMEAS_OP_OK;
	}
	return STOMEAS_OP_ERROR;
}


/**
 * @fn stomeas_res_t storage_meas_close_file(stomeas_handler_t * pHandler)
 * @brief Create a new measurement file
 * @param pHandler Pointer to the storage measurement handler
 * @return Result of the operation
 */
stomeas_res_t storage_meas_close_file(stomeas_handler_t * pHandler)
{
	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;

	BYTE buffer[2]; /* Buffer of the data to be written  */
	UINT bw = 0; /* Variable will be return the bytes written  */
	UINT btw = sizeof(uint16_t); /* Bytes to write in the file */

	buffer[0] = STOMEAS_END_OF_TEXT;
	buffer[1] = STOMEAS_END_OF_TRANSMISSION;

	/* Make sure to flush the file buffer */
	fr =  f_sync (&pHandler->file_handler);

	/* Place the read/write buffer at the end of the file */
	f_lseek(&pHandler->file_handler,
			f_size(&pHandler->file_handler));
	/* Before close the file */
	f_write(&pHandler->file_handler,
			buffer,
			btw,
			&bw);

	if((fr == FR_OK)  || (fr == FR_INVALID_OBJECT))
	{
		fr = f_close(&pHandler->file_handler);
		if ((fr == FR_OK)  || (fr == FR_INVALID_OBJECT))
		{
			return STOMEAS_OP_OK;
		}
	}

	return STOMEAS_OP_ERROR;
}

/**
 * @fn stomeas_res_t storage_meas_create_storage_data_block( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint8_t *buffe,r uint32_t elems)( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint8_t *buffer, uint32_t elems)
 * @brief Append a char array to the buffer specified
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param block_number number of the block
 * @param pDest pointer to the destination array
 * @param *buffer buffer to write in file
 * @param elems number of elements to write in file from the buffer
 * @return The size of the buffer
 */
uint32_t storage_meas_create_storage_data_block( uint32_t cycleCounter,
		uint8_t datasetStatus,
		uint8_t *buffer)
{
	buffer[0] = cycleCounter >> 24 & 0xff;
	buffer[1] = cycleCounter >> 16 & 0xff;
	buffer[2] = cycleCounter >> 8  & 0xff;
	buffer[3] = cycleCounter       & 0xff;
	buffer[4] = datasetStatus;
	return 5;
}

stomeas_res_t storage_meas_append_raw_data( stomeas_handler_t * pHandler, uint8_t *buffer, uint32_t elems)
{
	uint32_t bw = 0;

	if (!pHandler)
		return STOMEAS_OP_WRONG_PARAM;

	fr = f_lseek(&pHandler->file_handler, f_size(&pHandler->file_handler));
	if (fr == FR_OK)
	{
		fr = f_write(&pHandler->file_handler, buffer, elems, (unsigned int *) &bw);
		if (fr == FR_OK)
		{
			fr = f_sync (&pHandler->file_handler);
			if (fr == FR_OK)
			{
				return STOMEAS_OP_OK;
			}
		}
	}
	return STOMEAS_OP_ERROR;
}

/**
 * @fn stomeas_res_t storage_meas_set_file_name(uint32_t measurementId)
 * @brief Set the file name of a Default measurement file with the ID : 0
 * @return stomeas_res_t return the result of the operation as stomeas_rest_t enum type
 * @param uint32_t measureemntId the measurement ID
 */
stomeas_res_t storage_meas_set_file_name(uint32_t measurementId)
{
	if(measurementId == 0)
		return STOMEAS_OP_ERROR;

	FRESULT fr;

	char stoMeasFileName[32];
	int sizeOfStoMeasFileName = 0;

	/* Check the existence of the measurement file */
	fr = f_stat (STOMEAS_DEFAULT_FILE_NAME,
				&fno);

	if(fr == FR_OK)
	{
		/* The file exist */
		/* Parse the file name */
		sizeOfStoMeasFileName = storage_meas_parse_file_name(stoMeasFileName,
				32,
				measurementId);

		if(sizeOfStoMeasFileName)
		{
			/* Rename the file with his new name */
			fr = f_rename (STOMEAS_DEFAULT_FILE_NAME, stoMeasFileName);
			if(fr == FR_OK)
			{
				/* Rename operation succeed */
				return STOMEAS_OP_OK;
			}
		}
	}
	return STOMEAS_OP_ERROR;
}

/**
 * @fn int storage_meas_parse_file_name(char * name, size_t size, uint32_t measurementId)
 * @brief Parse the name of the storage measurement file
 * @param char * name [OUT]
 * @param size_t size [IN]
 * @param uint32_t measuermentId [IN]
 * return int number of character in the name buffer
 */
int storage_meas_parse_file_name(char *name, size_t size, uint32_t measurementId)
{
	int countChar = 0;
	countChar = snprintf(name, size, "m%06u.BIN", (unsigned int) measurementId);
	return countChar;
}
