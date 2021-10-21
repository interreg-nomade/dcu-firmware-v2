/*
 * rtos_fatfs.h
 *
 *  Created on: Dec 7, 2018
 *      Author: aclem
 */

#ifndef RTOS_FATFS_H_
#define RTOS_FATFS_H_

#include <stdbool.h>
#include <fatfs.h>

/**** Initialization prototype function ****/
FRESULT  FATFS_Init(void);
unsigned int rawConfigInit(void);

/**** RAW configuration related prototype function ****/
FRESULT rawConfFile_Open( BYTE rwAccessMode, bool append, bool createNewFile);
//FRESULT rawConfFile_Open(bool append);
unsigned int rawConfFile_Close(void);
unsigned int rawConfFile_Empty(void);
void rawConfFile_HandleNewConf(void);
unsigned int rawConfFile_Append(unsigned char * buffer, unsigned int size);
unsigned int rawConfigAppend(unsigned char * buffer, unsigned int n);
//unsigned int rawConfFile_Append(unsigned char * buffer, unsigned int size, bool sync);
unsigned int rawConfFile_RetrieveConf(void);
unsigned int rawConfigDecode(void);

/**** JSON configuration related prototype function ****/
FRESULT jsonConfFile_Open( BYTE rwAccessMode, bool append, bool createNewFile);
//FRESULT jsonConfFile_Open(bool append);
unsigned int jsonConfFile_Close(void);
unsigned int jsonConfFile_Empty(void);
void jsonConfFile_HandleNewConf(void);
unsigned int jsonConfFile_Append(char * buffer, unsigned int size);
//unsigned int jsonConfFile_Append(char* buffer, unsigned int size, bool append);
unsigned int fs_read_JsonConfFile(unsigned int offset, unsigned int elems, unsigned char *dest, unsigned int *destSize);
unsigned long fs_get_JsonConfFile_size(void);

/**** LOG file prototype function */
FRESULT logFile_Open(char * name);
unsigned int logFile_Close(void);
unsigned int logFile_Append(char* buffer, unsigned int size, bool sync);
FRESULT scan_files (char* path); /* Start node to be scanned (***also used as work area***) */

typedef enum {
	FILE_ERROR 			= -1,
	FILE_CLOSED	 		=  0,	/* File is closed */
	FILE_OPENED_EMPTY   =  1,	/* File is present, but empty */
	FILE_OPENED_EXISTS  =  2,	/* File is present, and contains data */
} SD_FILE_STATUS;

typedef enum {
	SD_NOT_INITIALIZED = 0,
	SD_MOUNTING,
	SD_MOUNTED,
	SD_ERROR
} SD_STATUS;

typedef struct {
	SD_STATUS status;

	SD_FILE_STATUS jsonConfigFileStatus;
	unsigned int jsonConfigSize;

	SD_FILE_STATUS rawConfigFileStatus;
	unsigned int rawConfigSize;

	SD_FILE_STATUS loggingFileStatus;
	unsigned int loggingFileSize;
} Status_SD;

#endif /* RTOS_FATFS_H_ */
