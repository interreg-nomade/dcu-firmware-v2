/*
 * rtos_fatfs.c
 *
 *  Created on: Dec 7, 2018
 *      Author: aclem
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 *
 */
#include <string.h>
#include <stdbool.h>

#include "board.h"
#include "interface_sd.h"
#include "fatfs.h"
#include "../app/queues/config_service/config_service_queue.h"
#include "../app/common.h"

#include "usart.h"  //to declare huart5
#include "../lib/tablet_com_protocol/fc_frames.h"

#define SD_JSON_CONFIG_FILE_NAME "JSONCONF.TXT"
#define SD_RAW_CONFIG_FILE_NAME  "CONFRAW.BIN"
#define SD_LOG_FILE_NAME         "LOGS.CSV"

#define INTERFACE_SD_DBG_PRINTF 0

extern char string[];
extern QueueHandle_t pPrintQueue;


__attribute__((section(".sd_objects")))  uint8_t sdRet; /* Return value for SD */
__attribute__((section(".sd_objects"))) char logicalPath[4]; /* SD logical drive path */
__attribute__((section(".sd_objects")))  FATFS FSObject; /* File system object for SD logical drive */

__attribute__((section(".sd_objects")))  FIL loggingFile; /* File object for SD */
__attribute__((section(".sd_objects")))  FIL jsonConfigFile; /* File object for SD */
__attribute__((section(".sd_objects")))  FIL rawConfigFile; /* File object for SD */

/* We need an intermediate buffer placed in RAM1 to operate in DMA */
__attribute__((section(".sd_objects")))  char jsonConfTmpBuf[2048];
/* We need an intermediate buffer placed in RAM1 to operate in DMA */
__attribute__((section(".sd_objects")))  char rawConfTmpBuf[1024];
/* We need an intermediate buffer placed in RAM1 to operate in DMA */
__attribute__((section(".sd_objects")))  char logConfTmpBuf1[2048];
/* We need an intermediate buffer placed in RAM1 to operate in DMA */
__attribute__((section(".sd_objects")))  char logConfTmpBuf2[2048];


//typedef struct {
//	SD_STATUS status;
//
//	SD_FILE_STATUS jsonConfigFileStatus;
//	unsigned int jsonConfigSize;
//
//	SD_FILE_STATUS rawConfigFileStatus;
//	unsigned int rawConfigSize;
//
//	SD_FILE_STATUS loggingFileStatus;
//	unsigned int loggingFileSize;
//} Status_SD;

Status_SD SD_Status;

FRESULT FATFS_Init(void) {

	FRESULT mountRes;
	/* Make sure the macro with the transceiver is set to 0, cubemx issue */
	/* Make sure the DMA macro template is uncommented */
	/* Make sure all the objects are placed in RAM1    */
	/* Turn on the SD card */
	SD_TurnOn();

	/*## FatFS: Link the SD driver ###########################*/
	retSD = FATFS_LinkDriver(&SD_Driver, logicalPath);

#if 0
	printf("[FATFS]Turning on SD CARD...\n");
	SD_TurnOff();
	osDelay(50);
	SD_TurnOn();
	printf(" ...done!\n");
	osDelay(200);
#endif

#if INTERFACE_SD_DBG_PRINTF
	sprintf(string, "[INTERFACE_SD] [FATFS_Init] Mounting...\n");
    xQueueSend(pPrintQueue, string, 0);
#endif
	SD_Status.status = SD_MOUNTING;

	mountRes = f_mount(&FSObject,        /* Mount the SD card */
			(TCHAR const*) logicalPath,  /* Path of the SD card drive */
			1);							 /* Mount the SD card now */

	if (mountRes == FR_OK)
	{
		/* Mounted the volume successfully */
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [FATFS_Init] Mounting success.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		SD_Status.status = SD_MOUNTED;
	}
	else
	{
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [FATFS_Init] Mounting failed.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		SD_Status.status = SD_ERROR;
	}
	return mountRes;
}

FRESULT rawConfFile_Open( 	BYTE rwAccessMode,
							bool append,
							bool createNewFile )
{
	FRESULT res;
	FILINFO fno;

	if(!createNewFile)
	{
		res = f_stat(SD_RAW_CONFIG_FILE_NAME, &fno); // Get File Status

		switch (res)
		{

		  	  case FR_OK:
#if INTERFACE_SD_DBG_PRINTF
//		  		  sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] Status RAW configuration file:\n", (unsigned int)HAL_GetTick());
					HAL_Delay(200);
		          xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Open] Status RAW configuration file:\n", 0);
		  		  sprintf(string, "		Size: %lu\n", fno.fsize);
					HAL_Delay(20);
		          xQueueSend(pPrintQueue, string, 0);
		  		  sprintf(string, "		Timestamp: %u/%02u/%02u, %02u:%02u\n",
		               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
		               fno.ftime >> 11, fno.ftime >> 5 & 63);
					HAL_Delay(20);
		          xQueueSend(pPrintQueue, string, 0);
		  		  sprintf(string, "		Attributes: %c%c%c%c%c\n",
		               (fno.fattrib & AM_DIR) ? 'D' : '-',
		               (fno.fattrib & AM_RDO) ? 'R' : '-',
		               (fno.fattrib & AM_HID) ? 'H' : '-',
		               (fno.fattrib & AM_SYS) ? 'S' : '-',
		               (fno.fattrib & AM_ARC) ? 'A' : '-');
					HAL_Delay(20);
		          xQueueSend(pPrintQueue, string, 0);
#endif
					if(append)
					{
						rwAccessMode |= FA_OPEN_APPEND;

#if INTERFACE_SD_DBG_PRINTF
//								sprintf(string, "%u[INTERFACE_SD] [rawConfFile_Open] [Append] Open raw file.\n", (unsigned int)HAL_GetTick());
								HAL_Delay(20);
								xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Open] [Append] Open raw file.\n", 0);
#endif

						res = f_open(&rawConfigFile, SD_RAW_CONFIG_FILE_NAME, rwAccessMode); // open file


						if(res == FR_OK)
						{
							if(!f_size(&rawConfigFile))
							{
								SD_Status.rawConfigFileStatus = FILE_OPENED_EMPTY; /* File is present, and contains NO data */
#if INTERFACE_SD_DBG_PRINTF
//								sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [Append] raw File is empty.\n", (unsigned int)HAL_GetTick());
								HAL_Delay(20);
								xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Open] [Append] raw File is empty.\n", 0);
#endif
							}
							else
							{
								SD_Status.rawConfigFileStatus = FILE_OPENED_EXISTS; /* File is present, and contains data */
#if INTERFACE_SD_DBG_PRINTF
//								sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [Append] raw File is present and contains data.\n", (unsigned int)HAL_GetTick());
								HAL_Delay(20);
								xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Open] [Append] raw File is present and contains data.\n", 0);
#endif
							}
						}
						else
						{
#if INTERFACE_SD_DBG_PRINTF
							sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [Append] res <> FR_OK. res = %d.\n", (unsigned int)HAL_GetTick(), res);
							HAL_Delay(20);
							xQueueSend(pPrintQueue, string, 0);
#endif
						}
					}
					else
					{
						rwAccessMode |= FA_OPEN_ALWAYS;

#if INTERFACE_SD_DBG_PRINTF
						sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [not Append] Open raw file.\n", (unsigned int)HAL_GetTick());
						HAL_Delay(20);
						xQueueSend(pPrintQueue, string, 0);
#endif

						res = f_open(&rawConfigFile, SD_RAW_CONFIG_FILE_NAME, rwAccessMode);

						if(res == FR_OK)
						{
							if(!f_size(&rawConfigFile))
							{
								SD_Status.rawConfigFileStatus = FILE_OPENED_EMPTY; /* File is present, and contains NO data */
#if INTERFACE_SD_DBG_PRINTF
								sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [not Append] File is empty.\n", (unsigned int)HAL_GetTick());
								HAL_Delay(20);
								xQueueSend(pPrintQueue, string, 0);
#endif
							}
							else
							{
								SD_Status.rawConfigFileStatus = FILE_OPENED_EXISTS; /* File is present, and contains data */
#if INTERFACE_SD_DBG_PRINTF
								sprintf(string, "%u [INTERFACE_SD] [rawConfFile_Open] [not Append] File is present and contains data.\n", (unsigned int)HAL_GetTick());
								HAL_Delay(20);
								xQueueSend(pPrintQueue, string, 0);
#endif
							}
						}
					}
		        break;

		    case FR_NO_FILE:
#if INTERFACE_SD_DBG_PRINTF
		    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Open] RAW configuration file does not exist.\n");
				HAL_Delay(20);
		        xQueueSend(pPrintQueue, string, 0);
#endif
				rwAccessMode |= FA_CREATE_NEW;
				res = f_open(&rawConfigFile,
						SD_RAW_CONFIG_FILE_NAME,
						rwAccessMode);
				if(res == FR_OK)
				{
					SD_Status.rawConfigFileStatus = FILE_OPENED_EMPTY;
				}
				else if (res == FR_EXIST)
				{
					rwAccessMode |= FA_CREATE_ALWAYS;
					res = f_open(&rawConfigFile,
							SD_RAW_CONFIG_FILE_NAME,
							rwAccessMode);
					if(res == FR_OK)
					{
						SD_Status.rawConfigFileStatus = FILE_OPENED_EMPTY;
					}
				}
		        break;

		    default:
#if INTERFACE_SD_DBG_PRINTF
		    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Open] An error occured. res = (%d)\n", res);
		        xQueueSend(pPrintQueue, string, 0);
#endif
		    	break;
		}

#if INTERFACE_SD_DBG_PRINTF
		        xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Open] After case...\n", 0);
#endif

	}
	else
	{
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [rawConfFile_Open] Create new RAW configuration file.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		f_unlink(SD_RAW_CONFIG_FILE_NAME);
		rwAccessMode |= FA_CREATE_ALWAYS;
		res = f_open(&rawConfigFile,
				SD_RAW_CONFIG_FILE_NAME,
				rwAccessMode);
		if(res == FR_OK)
		{
			SD_Status.rawConfigFileStatus = FILE_OPENED_EMPTY;
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [rawConfFile_Open] New RAW file opened successfully.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		f_stat(SD_RAW_CONFIG_FILE_NAME, &fno);
#if INTERFACE_SD_DBG_PRINTF
		  		  sprintf(string, "		Size: %lu\n", fno.fsize);
		          xQueueSend(pPrintQueue, string, 0);
		  		  sprintf(string, "		Timestamp: %u/%02u/%02u, %02u:%02u\n",
		               (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31,
		               fno.ftime >> 11, fno.ftime >> 5 & 63);
		          xQueueSend(pPrintQueue, string, 0);
		  		  sprintf(string, "		Attributes: %c%c%c%c%c\n",
		               (fno.fattrib & AM_DIR) ? 'D' : '-',
		               (fno.fattrib & AM_RDO) ? 'R' : '-',
		               (fno.fattrib & AM_HID) ? 'H' : '-',
		               (fno.fattrib & AM_SYS) ? 'S' : '-',
		               (fno.fattrib & AM_ARC) ? 'A' : '-');
		          xQueueSend(pPrintQueue, string, 0);
#endif
		}
	}
#if INTERFACE_SD_DBG_PRINTF
		        sprintf(string, "[INTERFACE_SD] [rawConfFile_Open] Before return res..., res value = 0x%04X\n", res);
				HAL_Delay(20);
		          xQueueSend(pPrintQueue, string, 0);
#endif

	return res;
}

unsigned int rawConfFile_Close(void)
{
	FRESULT res;

	if (SD_Status.rawConfigFileStatus <= 0)
	{
#if INTERFACE_SD_DBG_PRINTF
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Close] Error with RAW File Closing.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		return 0;
	}

	res = f_close(&rawConfigFile);

	if (res == FR_OK)
	{
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [rawConfFile_Close] RAW file Correctly closed.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
		SD_Status.rawConfigFileStatus = FILE_CLOSED;
	}
	else
	{
#if INTERFACE_SD_DBG_PRINTF
		sprintf(string, "[INTERFACE_SD] [rawConfFile_Close] RAW file closing error not further handled.\n");
        xQueueSend(pPrintQueue, string, 0);
#endif
			Error_Handler();
	}
	return 1;
}

unsigned int rawConfFile_Append(unsigned char * buffer, unsigned int size)
{
	FRESULT res;
	UINT wBytes;

	if (buffer == NULL || size == 0)
	{
#if INTERFACE_SD_DBG_PRINTF
	    xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Append] Error for RAW Writing: file is not opened or in error state\n", 0);
#endif
		return 0;
	}
	if (SD_Status.rawConfigFileStatus == FILE_CLOSED)
	{
		rawConfFile_Open(FA_WRITE, true, false);
	}
	else if(SD_Status.rawConfigFileStatus == FILE_OPENED_EXISTS || SD_Status.rawConfigFileStatus == FILE_OPENED_EMPTY)
	{
		rawConfFile_Close(); // Close the raw config file object for new action on it
#if INTERFACE_SD_DBG_PRINTF
		HAL_Delay(20);
	    xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Append] After RawConfFile_Close(), before rawConfFile_Open().\n", 0);
#endif
		HAL_Delay(500);
		rawConfFile_Open(FA_WRITE, true, false);
#if INTERFACE_SD_DBG_PRINTF
		HAL_Delay(20);
	    xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Append] After rawConfFile_open.\n", 0);
#endif
	}

	/* Init the buffer */
	memset(rawConfTmpBuf, 0, size);
	/* Copy the buffer to write in the tmp buf */
	memcpy(rawConfTmpBuf, buffer, size);
	/* Write data in the configuration file */
#if INTERFACE_SD_DBG_PRINTF
    sprintf(string, "[INTERFACE_SD] [rawConfFile_Append] Before f_write, size = 0x%08X\n", size);
	xQueueSend(pPrintQueue, string, 0);
	HAL_Delay(200);
    sprintf(string, "Contents rawConfTmpBuf = 0x");
  	char DUString[3];
    for (unsigned int i = 0; i < 46; i++)
    {
      sprintf(DUString, "%02X",rawConfTmpBuf[i]);
      strcat(string, DUString);
    }
    strcat(string, ".\n");
	HAL_Delay(20);
  	xQueueSend(pPrintQueue, string, 0);
    sprintf(string, "       Contents buffer = 0x");
    for (unsigned int i = 0; i < 46; i++)
    {
      sprintf(DUString, "%02X",buffer[i]);
      strcat(string, DUString);
    }
    strcat(string, ".\n");
	HAL_Delay(20);
  	xQueueSend(pPrintQueue, string, 0);
#endif
	HAL_Delay(500);

	res = f_write (
	  &rawConfigFile,       /* [IN] Pointer to the file object structure */
	  rawConfTmpBuf, 		/* [IN] Pointer to the data to be written */
	  size,         		/* [IN] Number of bytes to write */
	  &wBytes          		/* [OUT] Pointer to the variable to return number of bytes written */
	);

#if INTERFACE_SD_DBG_PRINTF
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Append] After f_write.\n");
	    xQueueSend(pPrintQueue, string, 0);
#endif


	if(res == FR_OK)
	{
		if(wBytes != size)
		{
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Close] Error in write of the raw configuration file: \n", 0);
			xQueueSend(pPrintQueue, "                                   size of the information received from the ANDROID Device is not equal to data in the file\n", 0);
			return 0;
		}
		else
		{
			/* Close the file we don't need it for the moment */
			rawConfFile_Close(); // Close the raw config file object for new action on it
			return 1;
		}
	}
	else
	{
		sprintf(string, "[INTERFACE_SD] [rawConfFile_Close] Error during the write operation of the configuration file\n");
		xQueueSend(pPrintQueue, string, 0);
		/* CLose the raw configuration file */
		rawConfFile_Close();
		return 0;
	}

	return 0;
}

void rawConfFile_HandleNewConf()
{
	if (SD_Status.rawConfigFileStatus == FILE_OPENED_EXISTS || SD_Status.rawConfigFileStatus == FILE_CLOSED)
	{
#if INTERFACE_SD_DBG_PRINTF
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_HandleNewConf] Deleting the existing RAW file to receive a new RAW configuration\n");
		xQueueSend(pPrintQueue, string, 0);
#endif
		rawConfFile_Empty();
	}
	else if ((SD_Status.rawConfigFileStatus == FILE_OPENED_EMPTY))
	{
    	/* File exists but is empty: fine */
#if INTERFACE_SD_DBG_PRINTF
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_HandleNewConf] Raw File exists but is empty.\n");
		xQueueSend(pPrintQueue, string, 0);
#endif
	}
	else
	{
#if INTERFACE_SD_DBG_PRINTF
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_HandleNewConf] Raw File error.\n");
		xQueueSend(pPrintQueue, string, 0);
#endif
		Error_Handler();
	}
}

unsigned int rawConfFile_Empty(void)
{
#if INTERFACE_SD_DBG_PRINTF
//    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Empty] Error in RAW file Writing: file is not opened\n");
		xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Empty] Starting...\n", 0);
#endif
    if(SD_Status.rawConfigFileStatus > 0)
	{
#if INTERFACE_SD_DBG_PRINTF
//    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Empty] Error in RAW file Writing: file is not opened\n");
		xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Empty] Start to close the raw file...\n", 0);
#endif
		rawConfFile_Close(); // Close the raw config file object for new action on it
	}
    else
    {
#if INTERFACE_SD_DBG_PRINTF
//    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Empty] Error in RAW file Writing: file is not opened\n");
    	sprintf(string, "[INTERFACE_SD] [rawConfFile_Empty] rawConfigFileStatus = %d\n",SD_Status.rawConfigFileStatus);
		xQueueSend(pPrintQueue, string, 0);
#endif

    }

	if(!rawConfFile_Open(FA_WRITE, false, true)) /* Open and do not append: force creation of a new file */
	{
		if (SD_Status.rawConfigFileStatus <= 0)
		{
#if INTERFACE_SD_DBG_PRINTF
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Empty] Error during opening RAW configuration file.\n", 0);
#endif
			return 0;
		}
#if INTERFACE_SD_DBG_PRINTF
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_Empty] New raw configuration file created.\n", 0);
#endif
			return 1;
	}

	return 0;
}

unsigned int rawConfFile_RetrieveConf(void)
{

  unsigned int fileSize = 0;
  unsigned int rbytes = 0;
  FRESULT fres = 0;
  FILINFO fno;

  memset(&fno, 0, sizeof(FILINFO));

//#if INTERFACE_SD_DBG_PRINTF
//	HAL_Delay(200);
//  xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_RetrieveConf] Started.\n", 0);
//#endif

  if(SD_Status.rawConfigFileStatus > 0)
  {
	rawConfFile_Close();
  }

  fres = rawConfFile_Open(FA_READ, false, false); /* Open the file in read mode and don't place the read write pointer at the end of the file */

	if(fres == FR_OK)
	{
		if(SD_Status.rawConfigFileStatus == FILE_OPENED_EXISTS)
		{
			fileSize = f_size(&rawConfigFile);
			if ((fileSize > 0) && (fileSize < RAW_CONFIG_BUFFER_SIZE))
			{
				/* Size is correct. */
				memset(rawConfTmpBuf, 0, 1024);
				fres = f_lseek(&rawConfigFile, 0);

				fres = f_read (
						&rawConfigFile,  /* [IN] pointer to the File object */
						rawConfTmpBuf,   /* [OUT] pointer to the data Buffer to store read data */
						fileSize,        /* [IN] Number of bytes to read */
						&rbytes          /* [OUT] pointer to number of bytes read */
				);

				if(fres == FR_OK)
				{
					if (rbytes != fileSize)
					{
#if INTERFACE_SD_DBG_PRINTF
						sprintf(string, "[INTERFACE_SD] [rawConfFile_RetrieveConf] RError : number of bytes that were read: %d/%d.\n", rbytes, fileSize);
						HAL_Delay(20);
						xQueueSend(pPrintQueue, string, 0);
#endif
						/* Close the file */
						rawConfFile_Close();
						/* Error handler */
						Error_Handler(); //todo : handle with app leds
						return 1;
					}
					else
					{
#if INTERFACE_SD_DBG_PRINTF
						HAL_Delay(20);
						xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_RetrieveConf] Retrieving RAW Configuration file.\n", 0);
#endif
						/* Close the file */
						rawConfFile_Close();
						/* Init the raw struct */
						rawConfigInit();
						/* Put the content of the file in the raw struct buffer */
						rawConfigAppend((unsigned char *)rawConfTmpBuf, rbytes);
						/* Return the result of the decoding */
						return rawConfigDecode();
					}
				}
				else
				{
#if INTERFACE_SD_DBG_PRINTF
				  HAL_Delay(20);
				  xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_RetrieveConf] Issue retrieving RAW Configuration file.\n", 0);
#endif
				}
			}
		}
		else if(SD_Status.rawConfigFileStatus == FILE_OPENED_EMPTY)
		{
			/* Close the file */
			rawConfFile_Close();
#if INTERFACE_SD_DBG_PRINTF
			HAL_Delay(20);
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfFile_RetrieveConf] RAW Configuration file opened empty.\n", 0);
#endif
			rawConfigInit();  /* Init the RAW config struct */
			return 2;
		}
	}

	/* Close the file */
	rawConfFile_Close();
	return 0;
}

unsigned int rawConfigDecode()
{
#if INTERFACE_SD_DBG_PRINTF
  xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfigDecode] Started.\n", 0);
#endif
  decode_result res;
  rawConfig.get();
  decodedConfig.get();
//  rawConfig.release();
//  decodedConfig.release();

  decodedConfig.state = CONF_RETRIEVING;

  memset(&decodedConfig.conf, 0, sizeof(decoded_config_t));

  res = decode_config((uint8_t*) rawConfig.buffer, (decoded_config_t *) &decodedConfig);
  rawConfig.release();
#if INTERFACE_SD_DBG_PRINTF
  sprintf(string, "[INTERFACE_SD] [rawConfigDecode] Raw configuration result = %0X.\n",res);
  xQueueSend(pPrintQueue, string, 0);
#endif
  if (res == DECODE_SUCCESS)
  {
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfigDecode] Raw configuration decoding done.\n", 0);
#endif
	decodedConfig.state = 	CONF_CORRECT;
	//print_config(decodedConfig.conf);
	/* Now linking the parsing function to each instruments, depending on their type */
	config_linkParsingFunctions(&decodedConfig.conf);

#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfigDecode] Link Parsing Functions done.\n", 0);
#endif
//	config_linkParsingFunctions_fromList(&decodedConfig.conf, parsingList, sizeof(parsingList)/sizeof(parsing_assoc_t));
//#if INTERFACE_SD_DBG_PRINTF
//	xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfigDecode] Link Parsing Functions from List done.\n", 0);
//#endif

	config_allocateDataSpace(&decodedConfig.conf);
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawConfigDecode] Allocating Data Space done.\n", 0);
#endif
	decodedConfig.release();

	return 1;
  }
  else
  {
#if INTERFACE_SD_DBG_PRINTF
	sprintf(string, "[INTERFACE_SD] [rawConfigDecode] Error while decoding the configuration, error code is: %d\n", res);
	xQueueSend(pPrintQueue, string, 0);
#endif
	decodedConfig.state = 	CONF_ERROR;
	decodedConfig.release();

//	Error_Handler();
	return 0;
  }
}

unsigned int rawConfigInit(void)
{
  rawConfig.get();
  memcpy(rawConfig.buffer, 0, sizeof(rawConfig.buffer));
  rawConfig.numElems = 0;
  rawConfig.release();
#ifdef INTERFACE_SD_DBG_PRINTF
  xQueueSend(pPrintQueue, "[INTERFACE_SD] [rawCongigInit] Memory Buffer for RAW file reserved.\n", 0);
#endif
  return 1;
}

unsigned int rawConfigAppend(unsigned char * buffer, unsigned int n)
{
  if (!buffer)
  { /* Error */
	return 0;
  }
  if (rawConfig.numElems + n >= RAW_CONFIG_BUFFER_SIZE)
  { /* Error */
	return 0;
  }
  memcpy((rawConfig.buffer + rawConfig.numElems), buffer, n);
  rawConfig.numElems += n;
  return 1;
}

unsigned int jsonConfFile_Append(char * buffer, unsigned int size)
{
    FRESULT res;
	UINT wBytes;
	if (buffer == NULL || size == 0)
	{
		return 0;
	}
	if (SD_Status.jsonConfigFileStatus == FILE_CLOSED)
	{
		jsonConfFile_Open(FA_WRITE, true, false);
	}
	else if(SD_Status.jsonConfigFileStatus == FILE_OPENED_EXISTS ||
			SD_Status.jsonConfigFileStatus == FILE_OPENED_EMPTY)
	{
		jsonConfFile_Close(); // Close the raw config file object for new action on it
		jsonConfFile_Open(FA_WRITE, true, false);
	}
	/* Init the buffer */
	memset(jsonConfTmpBuf, 0, size);
	/* Copy the buffer to write in the tmp buf */
	memcpy(jsonConfTmpBuf, buffer, size);
	/* Write data in the configuration file */
	res = f_write (
	  &jsonConfigFile,      /* [IN] Pointer to the file object structure */
	  jsonConfTmpBuf, 		/* [IN] Pointer to the data to be written */
	  size,         		/* [IN] Number of bytes to write */
	  &wBytes          		/* [OUT] Pointer to the variable to return number of bytes written */
	);

	if (res == FR_OK)
	{
		if (wBytes != size)
		{
#if INTERFACE_SD_DBG_PRINTF
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Append] Received JSON file size error.\n", 0);
#endif
			return 0;
		}
		else
		{
#if INTERFACE_SD_DBG_PRINTF
			sprintf(string, "[INTERFACE_SD] [jsonConfFile_Append] Writing %d to the JSON config file.\n", wBytes);
			xQueueSend(pPrintQueue, string, 0);
#endif
			/* Close the file we don't need it for the moment */
			jsonConfFile_Close(); // Close the raw config file object for new action on it
			return 1;
		}
	}
	else
	{
#if INTERFACE_SD_DBG_PRINTF
		xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Append] Error during writing operation.\n", 0);
#endif
		/* CLose the raw configuration file */
		jsonConfFile_Close();
		return 0;
	}
	return 0;
}

FRESULT jsonConfFile_Open( BYTE rwAccessMode, bool append, bool createNewFile)
{
  FRESULT res;
  FILINFO fno;
  if(!createNewFile)
  {
	res = f_stat(SD_JSON_CONFIG_FILE_NAME, &fno);
	switch (res)
	{
	case FR_OK:
	{
#if INTERFACE_SD_DBG_PRINTF
	  sprintf(string, "%u [INTERFACE_SD] [jsonConfFile_Open] JSON Configuration file:\n", (unsigned int)HAL_GetTick());
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "		Size: %lu\n", fno.fsize);
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "		Timestamp: %u/%02u/%02u, %02u:%02u\n", (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15, fno.fdate & 31, fno.ftime >> 11, fno.ftime >> 5 & 63);
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "		Attributes: %c%c%c%c%c\n",
		               (fno.fattrib & AM_DIR) ? 'D' : '-',
		               (fno.fattrib & AM_RDO) ? 'R' : '-',
		               (fno.fattrib & AM_HID) ? 'H' : '-',
		               (fno.fattrib & AM_SYS) ? 'S' : '-',
		               (fno.fattrib & AM_ARC) ? 'A' : '-');
	  xQueueSend(pPrintQueue, string, 0);
#endif
	  if(append)
	  {
		rwAccessMode |= FA_OPEN_APPEND;
		res = f_open(&jsonConfigFile, SD_JSON_CONFIG_FILE_NAME, rwAccessMode);
		if(res == FR_OK)
		{
		  if(!f_size(&rawConfigFile))
		  {
			SD_Status.jsonConfigFileStatus = FILE_OPENED_EMPTY; /* File is present, and contains NO data */
		  }
		  else
		  {
			SD_Status.jsonConfigFileStatus = FILE_OPENED_EXISTS; /* File is present, and contains data */
		  }
		}
	  }
	  else
	  {
		rwAccessMode |= FA_OPEN_ALWAYS;
		res = f_open(&jsonConfigFile, SD_JSON_CONFIG_FILE_NAME, rwAccessMode);
		if(res == FR_OK)
		{
		  if(!f_size(&jsonConfigFile))
		  {
			SD_Status.jsonConfigFileStatus = FILE_OPENED_EMPTY; /* File is present, and contains NO data */
#if INTERFACE_SD_DBG_PRINTF
		    xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Open] JSON File is present, but contains no data.\n", 0);
#endif
		  }
		  else
		  {
			SD_Status.jsonConfigFileStatus = FILE_OPENED_EXISTS; /* File is present, and contains data */
#if INTERFACE_SD_DBG_PRINTF
			xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Open] JSON File is present, and contains data.\n", 0);
#endif
		  }
		}
	  }
	  break;
	}
	case FR_NO_FILE:
	{
#if INTERFACE_SD_DBG_PRINTF
	  xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Open] JSON File didn't exist: creating it.\n", 0);
#endif
	  rwAccessMode |= FA_CREATE_NEW;
	  res = f_open(&jsonConfigFile, SD_JSON_CONFIG_FILE_NAME, rwAccessMode);
	  if(res == FR_OK)
	  {
		SD_Status.jsonConfigFileStatus = FILE_OPENED_EMPTY;
	  }
	  else if (res == FR_EXIST)
	  {
		rwAccessMode |= FA_CREATE_ALWAYS;
		res = f_open(&jsonConfigFile, SD_JSON_CONFIG_FILE_NAME, rwAccessMode);
		if(res == FR_OK)
		{
		  SD_Status.jsonConfigFileStatus = FILE_OPENED_EMPTY;
		}
	  }
	  break;
	}
	default:
	{
#if INTERFACE_SD_DBG_PRINTF
	  sprintf(string, "[INTERFACE_SD] [jsonConfFile_Open] An error occured. (%d)\n", res);
	  xQueueSend(pPrintQueue, string, 0);
#endif
	}
	}
  }
  else
  {
	f_unlink(SD_JSON_CONFIG_FILE_NAME);
	rwAccessMode |= FA_CREATE_ALWAYS;
	res = f_open(&jsonConfigFile, SD_JSON_CONFIG_FILE_NAME, rwAccessMode);
	if (res == FR_OK)
	{
	  SD_Status.jsonConfigFileStatus = FILE_OPENED_EMPTY;
	}
  }
  return res;
}

void jsonConfFile_HandleNewConf(void)
{
  if (SD_Status.jsonConfigFileStatus == FILE_OPENED_EXISTS || SD_Status.jsonConfigFileStatus == FILE_CLOSED)
  {
#if INTERFACE_SD_DBG_PRINTF
  	xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_HandleNewConf] Deleting the existing JSON file to receive a new configuration.\n", 0);
#endif
	jsonConfFile_Empty();
  }
  else if ((SD_Status.jsonConfigFileStatus == FILE_OPENED_EMPTY))
  { /* File exists but is empty: fine */
#if INTERFACE_SD_DBG_PRINTF
  	xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_HandleNewConf] File exists but is empty.\n", 0);
#endif
  }
  else
  {
	Error_Handler();
  }
}

unsigned int jsonConfFile_Close(void)
{
  FRESULT res;
  if (SD_Status.jsonConfigFileStatus <= 0)
  {
#if INTERFACE_SD_DBG_PRINTF
  	xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Close] Error for JSON Writing: file is not opened.\n", 0);
#endif
	return 0;
  }
  res = f_close(&jsonConfigFile);
  if (res == FR_OK)
  {
	SD_Status.jsonConfigFileStatus = FILE_CLOSED;
  }
  else
  {
	Error_Handler();
  }
  return 1;
}

unsigned int jsonConfFile_Empty(void)
{
#if INTERFACE_SD_DBG_PRINTF
  xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Empty] Cleaning the contents of the JSON configuration file...\n", 0);
#endif
  if (SD_Status.jsonConfigFileStatus > 0)
  {
#if INTERFACE_SD_DBG_PRINTF
  	xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Empty] Close the JSON configuration file before cleaning...\n", 0);
#endif
    jsonConfFile_Close();
  }
  if(!jsonConfFile_Open(FA_WRITE, false, true)) /* Open and do not append: force creation of a new file */
  {
	if(SD_Status.jsonConfigFileStatus <= 0)
	{
#if INTERFACE_SD_DBG_PRINTF
	  xQueueSend(pPrintQueue, "[INTERFACE_SD] [jsonConfFile_Empty] Error for JSON Writing: file is not opened.\n", 0);
#endif
	  return 0;
	}
	return 1;
  }
  return 0;
}

unsigned long fs_get_JsonConfFile_size(void)
{
  if (SD_Status.status == SD_MOUNTED)
  {
	if (SD_Status.jsonConfigFileStatus == FILE_OPENED_EXISTS)
	{
	  FRESULT fres = 0;
	  FILINFO fno;
	  unsigned long length =0;
	  memset(&fno, 0, sizeof(FILINFO));
	  fres = f_stat(SD_JSON_CONFIG_FILE_NAME, &fno);
	  if (fres == FR_OK)
	  {
		length = fno.fsize;
		return length;
	  }
	  else
	  {
		return 0;
	  }
	}
	else
	{
	  return 0;
	}
  }
  else
  {
	return 0;
  }
}

unsigned int fs_read_JsonConfFile(unsigned int offset, unsigned int elems, unsigned char *dest, unsigned int *destSize)
{
  if ( (!dest) || (!destSize) )
  { /* Pointer test fail */
	return 0;
  }
  FRESULT fres = 0;
  unsigned int rbytes = 0;
#if INTERFACE_SD_DBG_PRINTF
  sprintf(string, " [INTERFACE_SD] [fs_read_JsonConfFile] [%s] have to read %d starting from %d\n", __func__, elems, offset);
  xQueueSend(pPrintQueue, string, 0);
#endif
  /* Place file offset */
  fres = f_lseek(&jsonConfigFile, offset);
  if (fres != FR_OK)
  { /* Different than FR_OK - Error to handle */
	return 0;
  }
  fres = f_read (
		&jsonConfigFile,  	/* [IN] File object */
		jsonConfTmpBuf,   	/* [OUT] Buffer to store read data */
		elems,        		/* [IN] Number of bytes to read */
		&rbytes          	/* [OUT] Number of bytes read */
			    );
  if (fres != FR_OK)
  { /* Different than FR_OK - Error to handle */
	return 0;
  }
  if (rbytes != elems)
  { /* Error */
	return 0;
  }
  if (rbytes >= 2048)
  {
	return 0;
  }
  /* Copy from buffer accessible by SDIO in DMA mode, to the destination */
  memcpy(dest, jsonConfTmpBuf, rbytes);
  *destSize = rbytes;
  /* Success */
  return 1;
}

__attribute__((section(".sd_objects"))) FRESULT res;
__attribute__((section(".sd_objects"))) DIR dir;
__attribute__((section(".sd_objects"))) UINT i;
__attribute__((section(".sd_objects"))) static FILINFO fno;

FRESULT scan_files(char* path)        /* Start node to be scanned (***also used as work area***) */
{
#if INTERFACE_SD_DBG_PRINTF
  sprintf(string, "Scanning path %s: \n", path);
  xQueueSend(pPrintQueue, string, 0);
#endif
  res = f_opendir(&dir, path);                     /* Open the directory */
  if (res == FR_OK)
  {
    for (;;)
    {
      res = f_readdir(&dir, &fno);                   /* Read a directory item */
      if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
      if (fno.fattrib & AM_DIR)                      /* It is a directory */
      {
        i = strlen(path);
#if INTERFACE_SD_DBG_PRINTF
        sprintf(&path[i], "		/%s", fno.fname);
  		xQueueSend(pPrintQueue, string, 0);
#endif
        res = scan_files(path);                      /* Enter the directory */
        if (res != FR_OK) break;
        path[i] = 0;
      }
      else                                          /* It is a file. */
      {
#if INTERFACE_SD_DBG_PRINTF
        sprintf(string, "		%s/%s\n", path, fno.fname);
  		xQueueSend(pPrintQueue, string, 0);
#endif
      }
    }
    f_closedir(&dir);
  }
  return res;
}

FRESULT logFile_Open(char * name) /* name length as a safety to avoid overflow */
{
  FRESULT res;
  FILINFO fno;
  DIR dir;
  bool    exists = false;
  if (!strnlen(name, 24))
  {
	/* return 0 */
	/* error to handle */
  }
  /* else name is correct */
  scan_files("/logs");
  res = f_opendir(&dir, "/logs");
  res = f_stat(name, &fno);
  switch (res)
  {
    case FR_OK:
    {
#if INTERFACE_SD_DBG_PRINTF
	  sprintf(string, "Logging file stats: \n");
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "Size: %lu bytes\n", fno.fsize);
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "Timestamp: %u/%02u/%02u, %02u:%02u\n",
					   (fno.fdate >> 9) + 1980, fno.fdate >> 5 & 15,
					   fno.fdate & 31, fno.ftime >> 11, fno.ftime >> 5 & 63);
	  xQueueSend(pPrintQueue, string, 0);
	  sprintf(string, "Attributes: %c%c%c%c%c\n",
					   (fno.fattrib & AM_DIR) ? 'D' : '-',
					   (fno.fattrib & AM_RDO) ? 'R' : '-',
					   (fno.fattrib & AM_HID) ? 'H' : '-',
					   (fno.fattrib & AM_SYS) ? 'S' : '-',
					   (fno.fattrib & AM_ARC) ? 'A' : '-');
	  xQueueSend(pPrintQueue, string, 0);
#endif
	  exists = true;
	  break;
    }
    case FR_NO_FILE:
    {
#if INTERFACE_SD_DBG_PRINTF
	  xQueueSend(pPrintQueue, "File does not exist.\n", 0);
#endif
	  break;
    }
    default:
    {
#if INTERFACE_SD_DBG_PRINTF
	  sprintf(string, "An error occured. (%d)\n", res);
	  xQueueSend(pPrintQueue, string, 0);
#endif
    }
  }
  unsigned int cx = 0;
  unsigned char filenameWithPath[64];
  memset(filenameWithPath, 0, 64);
  cx = snprintf((char *)filenameWithPath, 64, "logs/%s", name);
#if INTERFACE_SD_DBG_PRINTF
  sprintf(string, "filename with path: %s\n", filenameWithPath);
  xQueueSend(pPrintQueue, string, 0);
#endif
  if (exists)
  {
	res = f_open(&loggingFile, (const TCHAR *)filenameWithPath, FA_OPEN_APPEND | FA_WRITE | FA_READ);
  }
  else
  {
	res = f_open(&loggingFile, (const TCHAR *)filenameWithPath, FA_CREATE_ALWAYS | FA_WRITE | FA_READ);
  }
  switch (res)
  {
	case FR_OK:
	{
#if INTERFACE_SD_DBG_PRINTF
	  xQueueSend(pPrintQueue, "Operation OK.\n", 0);
#endif
	  break;
	}
	default:
	{ /* Error occured */
	  Error_Handler();
	}
  }
  return res;
}

unsigned int logFile_Close()
{
  FRESULT res;
  if (SD_Status.rawConfigFileStatus <= 0)
  {
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "Error for RAW CLOSING: file is not opened.\n", 0);
#endif
	return 0;
  }
  res = f_close(&rawConfigFile);
  if (res == FR_OK)
  {
	SD_Status.rawConfigFileStatus = FILE_CLOSED;
  }
  else
  {
	Error_Handler();
  }
  return 1;
}

unsigned int logFile_Append(char* buffer, unsigned int size, bool sync)
{
  volatile FRESULT res;
  if (!buffer)
  {
    return 0;
  }
  if (SD_Status.rawConfigFileStatus <= 0)
  {
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "Error for RAW Writing: file is not opened or in error state.\n", 0);
#endif
    return 0;
  }
  rawConfTmpBuf[0] = 0;
  /* Copy the buffer to write in the tmp buf */
  memcpy(logConfTmpBuf1, buffer, size);
  res = f_write (&loggingFile,   /* [IN] Pointer to the file object structure */
	             logConfTmpBuf1, /* [IN] Pointer to the data to be written */
	             size,         	 /* [IN] Number of bytes to write */
	             NULL          	 /* [OUT] Pointer to the variable to return number of bytes written */
	            );
  if (res != FR_OK)
  {
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "Error in write\n", 0);
#endif
	Error_Handler();
  }
  if (sync)
  {
#if INTERFACE_SD_DBG_PRINTF
	xQueueSend(pPrintQueue, "Log sync.\n", 0);
#endif
    res = f_sync (&loggingFile);  /* [IN] File object */
	if (res != FR_OK)
	{
#if INTERFACE_SD_DBG_PRINTF
	  xQueueSend(pPrintQueue, "Error in sync.\n", 0);
#endif
	  Error_Handler();
	}
	else
	{ /* op ok */
	  logConfTmpBuf1[0] = 0;
	}
  }
  return 1;
}
