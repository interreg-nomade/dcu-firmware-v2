/**
 * @file common.h
 * @brief Contain ressources used by all the applications.
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef COMMON_H_
#define COMMON_H_

#include <stdlib.h>
#include "config/nodes.h"
#include "config/parameters_id.h"
#include "config/parsing.h"
#include "config/config_op.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "config/raw.h"

#include "data/structures.h"
#include "data/operations.h"

void common_config_init();
unsigned int getConfigId();
unsigned int getConfigVersion();

#define MAX_ASSOCIATIONS 32

#define CONFIG_WAITING_FOR_DECODE() \
do { 			\
	osDelay(10); \
} while (decodedConfig.state != CONF_CORRECT)

#define CONF_IS_DECODED() (decodedConfig.state == CONF_CORRECT)

#define RAW_CONFIG_BUFFER_SIZE 1024
#define RAM1_PLACE	__attribute__((section(".RAM1_OBJ")))

typedef enum{
	CONF_NOT_READY=0,
	CONF_READY,
	CONF_CORRECT,
	CONF_RETRIEVING,
	CONF_BUILDING,
	CONF_ERROR,
}conf_state_t;

typedef struct {
	unsigned char buffer[RAW_CONFIG_BUFFER_SIZE];
	unsigned int  numElems;
	unsigned char * ptr;

	conf_state_t status;

	SemaphoreHandle_t mutex;

	unsigned int (*get)    (void);
	int 		 (*release)(void);

} protectedRawConfig_t;

typedef struct {
	decoded_config_t conf;
	conf_state_t state;
	SemaphoreHandle_t mutex;

	unsigned int (*get)    (void);
	void		 (*release)(void);

} protectedDecodedConfig_t;

typedef struct {
	unsigned int JsonOnFatFS;
	unsigned int rawOnFatFS;

} configStatus_t;

extern RAM1_PLACE protectedRawConfig_t 		rawConfig;
extern RAM1_PLACE protectedDecodedConfig_t 	decodedConfig;
extern RAM1_PLACE decoded_config_t 			snapshotconf;

extern  node_decriptor nodes[];
extern  parsing_assoc_t parsingList[MAX_ASSOCIATIONS];

#endif /* COMMON_H_ */
