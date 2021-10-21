/**
 * @file config_op.h
 * @brief Contains all the operations related to the configuration
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
#ifndef CONFIG_CONFIG_OP_H_
#define CONFIG_CONFIG_OP_H_

#include "../data/operations.h" /* Contains the parsing functions */
#include "raw.h"
#include "parsing.h"

/* Scan the configuration, and delivers a buffer with all the measurements parsed */
/* pConf: pointer to the config */
/* buffer: pointer to the destination buffer */
/* n: number of bytes processed */
int config_parseAllInstruments(decoded_config_t *pConf, unsigned char * buffer, unsigned int n);
/* Do the link job between instrument type and parsing functions */
int config_linkParsingFunctions(decoded_config_t * pConf);
/* Allocate memory for the data pointers */
int config_allocateDataSpace(decoded_config_t * pConf);

/* pConf pointer to the decoded configuration
 * dest pointer to the destination buffer
 * n pointer to an unsigned int ; number of bytes writtens
 */
int config_createStreamPacket(decoded_config_t * pConf, unsigned char * pDest, unsigned int * pSize);

int config_createStoragePacket(decoded_config_t * pConf,
		unsigned char * pDest,
		unsigned int * pSize);

int getInstrumentFromConfig(decoded_config_t * config, instrument_config_t ** pConf, int type);
int getNumberOfInstrumentSpecificFromConfig(decoded_config_t * conf, int type);
int config_linkParsingFunctions_fromList(decoded_config_t * pConf, parsing_assoc_t * list, unsigned int nNodes);

int config_copy(decoded_config_t * pDest, decoded_config_t * pSource);

unsigned int config_getStoragePacketSize(decoded_config_t * pConf);

#endif /* CONFIG_CONFIG_OP_H_ */
