/**
 * @file storage_measurements.h
 * @brief Measurement storage library.
 * @author YNCREA HDF / TEAM ISEN Robotics
 * @version 0.1
 * @date 23 August 2019
 *
 * The purpose is to save measurements in an organized way, and to provide tools to manage it (create, close, read, write, append, ...)
 *
 */

#ifndef STORAGE_STOMEAS_LIB_STORAGE_MEASUREMENTS_H_
#define STORAGE_STOMEAS_LIB_STORAGE_MEASUREMENTS_H_

/* Includes */
#include "stdint.h"
#include "fatfs.h"  /* File system */
#include "config/raw.h" /* EDUCAT Configuration API is required */
#include <stdbool.h>

#define RAM_PLACEMENT __attribute__((section(".sd_objects")))

#define STOMEAS_DEFAULT_FILE_NAME "m000000.BIN"
/** Definitions */
#define STOMEAS_HEADER_SIZE 30/*!< Size of the header, in bytes, !with the Start of heading and start of text  */
#define STOMEAS_START_OF_HEADING 		 0x01
#define STOMEAS_START_OF_TEXT	 		 0x02
#define STOMEAS_END_OF_TEXT		 	 	 0x03
#define STOMEAS_END_OF_TRANSMISSION		 0x04

#define STOMEAS_HANDLER_BUFFER_SIZE 1024

/** Storage measurement operation/status result */
typedef enum
{
	STOMEAS_OP_ERROR_BUFFER_USED = -8,
	STOMEAS_OP_FILE_CORRUPT = -7,
	STOMEAS_OP_FILE_DOES_NOT_EXIST = -6,
	STOMEAS_OP_FILE_EXISTS_WRONG_OP = -5,
	STOMEAS_OP_BLOCK_NOT_FOUND = -4,
	STOMEAS_OP_SPECIFIED_FILE_UNKNOWN = -3, /*!< The file specified is not present, mainly for files search */
	STOMEAS_OP_TIMESTAMP_OUT_OF_RANGE = -2, /*!< The timestamp specified is out of range */
    STOMEAS_OP_WRONG_PARAM = -1,			/*!< One of the specified parameter is wrong */
    STOMEAS_OP_ERROR = 0,					/*!< Operation resulted in an error */
    STOMEAS_OP_OK = 1,						/*!< Operation complete without errors */
    STOMEAS_OP_PENDING = 2, 				/*!< Operation is still pending */
	STOMEAS_OP_BLOCK_FOUND = 3,
	STOMEAS_OP_FILE_EXISTS = 4
} stomeas_res_t;


/**
 * @struct stomeas_handler_t
 * @brief Measurement's storage handler.
 *
 * Contains informations necessary to run the library properly (file handler, current pointer, state, ...)
 */
typedef struct {
	FILINFO file_state;					/*!< Informations about the measurement file */
	FIL file_handler;					/*!< Measurement file handler */
	uint64_t file_current_pointer;		/*!< Position of the pointer, used to navigate through the measurement file. */
	uint8_t buffer[STOMEAS_HANDLER_BUFFER_SIZE];
	uint32_t buffer_elems;
	bool bufferUsed;
	bool valid;
} stomeas_handler_t;


/**
 * @struct stomeas_header_t
 * @brief Header of a measurement file as structure
 *
 * Contains informations contained in the header of a measurement file.
 */
typedef struct {
	uint16_t setup_id; 			/*!< Setup ID of a configuration */
	uint16_t version;			/*!< Version of a configuration */
	uint16_t company_id;		/*!< Company ID contained in a configuration */
	uint32_t measurement_id;	/*!< ID of a measurement */
	uint64_t start_time;		/*!< Start time of a measurement (Unix Epoch, in ms) */
	uint64_t stop_time;			/*!< Stop time of a measurement (Unix Epoch, in ms) */
	uint16_t block_length;		/*!< Length of a block inside a specific measurement file */
	bool valid;
} stomeas_header_t;


stomeas_res_t storage_meas_create_file(stomeas_handler_t * pHandler, char * array, uint8_t clear);
stomeas_res_t storage_meas_set_file(stomeas_handler_t * pHandler, char * array, uint32_t array_len);
stomeas_res_t storage_meas_check_file_exists(char * array);
stomeas_res_t storage_meas_get_header( stomeas_handler_t * pHandler, stomeas_header_t * pHeader);
stomeas_res_t storage_meas_set_header( stomeas_handler_t * pHandler, stomeas_header_t * pHeader);
stomeas_res_t storage_meas_close_file( stomeas_handler_t * pHandler);



stomeas_res_t storage_meas_get_file_infos(FILINFO * pInfos, char * array, uint32_t array_len );
stomeas_res_t storage_meas_search_block_from_timestamp( stomeas_handler_t * pHandler,  stomeas_header_t *pHeader, uint64_t timestamp);
stomeas_res_t storage_meas_get_block( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint64_t block_number, uint8_t * pDest);
stomeas_res_t storage_meas_append_block( stomeas_handler_t * pHandler, stomeas_header_t *pHeader);
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

stomeas_res_t storage_meas_util_verify_integrity_size( stomeas_handler_t * pHandler, stomeas_header_t *pHeader);
stomeas_res_t storage_meas_util_fill_zeros( stomeas_handler_t * pHandler, stomeas_header_t *pHeader);
stomeas_res_t storage_meas_util_build_block_header( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint64_t epochms, uint16_t measurementStatus);

stomeas_res_t storage_meas_fill_gap( stomeas_handler_t * pHandler, stomeas_header_t * pHeader, uint32_t ccNow );
uint32_t storage_meas_get_number_blocks_in_file( stomeas_handler_t * pHandler, stomeas_header_t * pHeader );
/**
 * @fn stomeas_res_t storage_meas_append_data_block_dbuffer( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint8_t *buffer, uint32_t elems)
 * @brief Append a char array to the measurement file.
 *
 * @param pHandler Pointer to the storage measurement handler
 * @param pHeader Pointer to a measurement's header
 * @param block_number number of the block
 * @param pDest pointer to the destination array
 * @param *buffer buffer to write in file
 * @param elems number of elements to write in file from the buffer
 * @return Result of the operation
 */
stomeas_res_t storage_meas_append_data_block_dbuffer( stomeas_handler_t * pHandler, stomeas_header_t *pHeader, uint8_t *buffer, uint32_t elems);
uint32_t storage_meas_create_storage_data_block( uint32_t cycleCounter,		uint8_t datasetStatus, uint8_t *buffer);
stomeas_res_t storage_meas_append_raw_data( stomeas_handler_t * pHandler, uint8_t *buffer, uint32_t elems);

stomeas_res_t storage_meas_set_file_name(uint32_t measurementId);
int storage_meas_parse_file_name(char *name, size_t size, uint32_t measurementId);
#endif /* STORAGE_STOMEAS_LIB_STORAGE_MEASUREMENTS_H_ */
