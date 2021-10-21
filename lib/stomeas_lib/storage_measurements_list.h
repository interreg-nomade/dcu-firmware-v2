/**
 * @file storage_measurements_list.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date February 2020
 * @project Interreg EDUCAT
 */


#ifndef STOMEAS_LIB_STORAGE_MEASUREMENTS_LIST_H_
#define STOMEAS_LIB_STORAGE_MEASUREMENTS_LIST_H_

#include <stdint.h>
#include "common.h"

#define MEASUREMENT_LISTS_MAX_ELEMS 32

typedef enum {
	measurement_list_ressource_busy,
	measurement_list_ressource_available,
	measurement_list_correct

} measurement_list_status;

typedef struct {
	uint32_t id;		/* Measurement ID */
	uint64_t startTime; /* Measurement start time (unix epoch) */
	uint64_t stopTime;  /* Measurement stop  time (unix epoch) */
} measurement_struct_t;

typedef struct {
	uint16_t setupID;
	uint16_t userID;
	uint16_t version;
	uint16_t companyID;

	measurement_struct_t list[MEASUREMENT_LISTS_MAX_ELEMS];
	uint32_t elems; /* Elements in the list */

	measurement_list_status status;

} measurements_list_t;

typedef enum {
	meas_list_op_wrong_param,
	meas_list_op_ok,
	meas_list_op_err,
	meas_list_not_imp,
	meas_list_no_possibility

} measurement_list_op_result;

extern measurements_list_t measurementsList; /* Let tablet com app access this ressource */

measurement_list_op_result measurement_list_init(measurements_list_t * plist);
measurement_list_op_result measurement_list_read_file();
//measurement_list_op_result measurement_list_read_file(uint32_t sizeOfFile);

measurement_list_op_result measurement_list_write_file();
measurement_list_op_result measurement_list_add(measurement_struct_t * pmeas);
measurement_list_op_result measurement_list_remove(measurement_struct_t * pmeas);
measurement_list_op_result measurement_list_destroy(measurement_struct_t * pmeas);

measurement_list_op_result measurement_list_interpret_data_from_tablet(uint8_t * buffer);
measurement_list_op_result measurement_list_prepare_data_for_sd_write(decoded_config_t * pConf,
																	  measurements_list_t * pMeasurementsList,
																	  uint32_t * nBytes);

unsigned int measurement_list_file_append(unsigned char * buffer, unsigned int size);
#endif /* STOMEAS_LIB_STORAGE_MEASUREMENTS_LIST_H_ */
