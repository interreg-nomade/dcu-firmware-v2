/**
 * @file app_storage.c
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date September, October 2019
 */
#include <stdbool.h>
#include <string.h>

#include "FreeRTOS.h"
#include "cmsis_os.h"

#include "common.h"

#include "app_storage.h"
#include "app_tablet_com.h"
#include "app_rtc.h"
//#include "app_gps.h"
#include "usart.h"


#include "stomeas_lib/storage_measurements.h"
#include "stomeas_lib/storage_measurements_list.h"
#include "stomeas_lib/storage_ring_buffer.h"

#include "queues/measurements/measurements_queue.h"
#include "queues/sd_writer_service_queue/sd_writer_service_queue.h"
#include "queues/android_device_com/android_com_queue.h"

#define PRINT_APP_STORAGE_DBG_MSG 1

#define BIG_BUFFER_SIZE 128000

/* TASKS Handler */
static osThreadId measStorageThreadHandler;
static osThreadId sdWriterThreadHandler;

static bool storageEnabled = false;

static unsigned int correctMeasurementFiles = 0;
static uint32_t indexFile      = 0;

static measurement_ring_block tempBlockFlush;
static uint8_t flag_WriterBusy = 0;

static RAM_PLACEMENT stomeas_handler_t lHandler;
static stomeas_header_t lHeader;

/* The measurement list stores a list of time intervals, linked to a measurement ID, along with a configuration header */
RAM_PLACEMENT measurements_list_t measurementsList;

/* Measurement blocks ring buffer */
RAM_PLACEMENT mes_ring_buffer_t measRingBuffer;
RAM_PLACEMENT measurement_ring_block temporaryBlock;

/* Big F****** Buffer for the sdWriter Thread */
RAM_PLACEMENT uint8_t bigBuffer[BIG_BUFFER_SIZE];
RAM_PLACEMENT uint32_t bigBufferElems = 0;

/* The ring buffer stores a defined amount of measurements and is used to satisfy periodicity on the measurements
 * -> the app_storage pushes measurements block in the ring buffer
 * -> the app_storage_sync pull measurements block from ring buffer, parse it and write it on SD Card  */

/***** Private functions *****/
static void measStorageThread(const void * params);
static void measStorWriteThread(const void * params);

static void set_storage_flag(bool value);
static bool get_storage_flag(void);

static bool measurement_range_valid(uint64_t startTime, uint64_t stopTime, uint64_t now);
static measurement_struct_t get_active_measurement(measurementsServiceQueueMsg_t * serviceQueueMsg);

static uint32_t reset_bigBuffer_elems(void);
static uint32_t get_bigBuffer_elems(void);
static void set_writer_flag(bool value);

static void app_sd_writer_notify();

static int app_storage_open_new_file(uint32_t measurmentId);

static void app_storage_find_measurement_file(void);

extern char string[];
extern QueueHandle_t pPrintQueue;

/**
 * @fn int rtos_measurement_storage_init(void)
 * @brief Initialization of the tasks and the queue using in the storage process
 * @param
 * @return
 */
int rtos_measurement_storage_init(void)
{
  osThreadDef(measStorage, measStorageThread, osPriorityNormal, 0, 512);	/* Initialization of measStorageThread */
  measStorageThreadHandler = osThreadCreate (osThread(measStorage), NULL);	/* Start measStorageThread */
  osThreadDef(sdWriter, measStorWriteThread, osPriorityNormal, 0, 512);		/* Initialization of measStorWriteThread */
  sdWriterThreadHandler = osThreadCreate (osThread(sdWriter), NULL);		/* Start of measStorWriteThread */
  mes_ring_buffer_init(&measRingBuffer);									/* Initialization of measurement ring buffer */
  sdwriter_service_queue_init();											/* Initialize communication queue between SD writer and measurement task */
  return 1;
}

/**
 * @fn static void measStorageThread(const void * params)
 * @brief This task checks from the measurement list file if an automated measurement can be started
 * also it checks if a measurement is started by the android device by checking the information coming from
 * the communication queue using the communication thread of the android device and this task
 * @param
 * @return
 */
static void measStorageThread(const void * params)
{
  uint32_t ulNotificationValue   = 0; 						/* Used for notification of the thread */
  const TickType_t xMaxBlockTime = pdMS_TO_TICKS(200);
  uint64_t epochNow_Ms           = 0; 						/* Used to keep Epoch time */
  uint32_t flushcounter          = 0; 						/* Flush counter used for notification of SD writer thread */
  measurement_list_op_result       measRes;
  measurement_struct_t             activeMeasurement;
  measurementsServiceQueueMsg_t    serviceQueueMsg;
  memset(&activeMeasurement, 0, sizeof(measurement_struct_t));
  memset(&serviceQueueMsg,   0, sizeof(measurementsServiceQueueMsg_t));
  memset(&lHeader,           0, sizeof(stomeas_header_t));
  memset(&lHandler,          0, sizeof(stomeas_handler_t));
  while ((decodedConfig.state != CONF_CORRECT) && (rtcUnsynchronized != 0)) /* Wait until configuration is decoded and RTC is synchronized */
  {
	osDelay(20);
  }
  measRes = measurement_list_init(&measurementsList); /* Initialize measurement list structure, read measurement list file and store it if not out of date */
  if(measRes == meas_list_op_ok)
  {
	if (measurementsList.status == measurement_list_correct)
	{
	  activeMeasurement = get_active_measurement(&serviceQueueMsg); /* Check for an active measurement in measurement file */
	}
  }
  xTaskNotifyStateClear( NULL ); /* Ensure the calling task's notification state is not already pending. */
  for(;;)
  {
	if(activeMeasurement.id == 0 && serviceQueueMsg.status == measurement_StatusOriginIDLE)
	{  /* No active measurement and no measurement was launched before */
	  activeMeasurement = get_active_measurement(&serviceQueueMsg); /* Check for an active measurement in measurement file */
	  if(activeMeasurement.id == 0)
	  { /* No active measurement was found in the measurement list */
		if(measurements_service_receive(&serviceQueueMsg))
		{ /* Received a message from tablet_com task queue */
		  if(serviceQueueMsg.status == measurement_StatusOriginAndroidDevice)
		  {
			if(serviceQueueMsg.action == measurement_StartMeasurement)
			{
#if PRINT_APP_STORAGE_DBG_MSG
		      xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] Storage file created and storage flag set.\n", 0);
#endif
			  storage_meas_create_file(&lHandler, STOMEAS_DEFAULT_FILE_NAME, 1); 	/* Open the file and create it */
			  set_storage_flag(true); 												/* Set storage flag */
			}
		  }
		}
	  }
	}
	else if(serviceQueueMsg.status == measurement_StatusOriginAndroidDevice)
	{
	  activeMeasurement = get_active_measurement(&serviceQueueMsg); /* Check for an active measurement in measurement file */
	  if(activeMeasurement.id == 0)
	  {
		measurements_service_receive(&serviceQueueMsg); /* Received a message from tablet_com task queue */
		/* Normally the only expected request is to stop the measurement */
		/* Check action if it is related to set measurement ID */
		if(serviceQueueMsg.status == measurement_StatusOriginAndroidDevice)
		{
		  if(serviceQueueMsg.action == measurement_SetMeasurementID)
		  { /* We received measurement ID of actual measurement from android device */
		    serviceQueueMsg.action = measurement_StartMeasurement; /* Put action status back to Start Measurement */
		  }
		}
	  }
	}
	if (xTaskNotifyWait( 0x00,					/* Don't clear any bits on entry. */
				         0xffffffff,			/* Clear all bits on exit. (long max) */
				         &ulNotificationValue,	/* Receives the notification value. */
				         xMaxBlockTime ))		/* Block 200 ms */
	{
	  if ((ulNotificationValue & APP_STORAGE_NOTIF_CYCLE_COUNTER) == APP_STORAGE_NOTIF_CYCLE_COUNTER)
	  { /* Received notification from syncThread (app_sync.c). This happens every 20ms. */
		/* syncThread is in charge to make a copy of decoded configuration structure which contains the data from the different instruments */
		app_rtc_get_unix_epoch_ms(&epochNow_Ms); 			/* Get the epoch time */
		uint32_t cycleCounter = snapshotconf.cycleCounter; 	/* Copy the cycle counter */
		if(serviceQueueMsg.status == measurement_StatusOriginMeasurementList)
		{ /* An automated measurement is requested from measurement list file */
		  if (tablet_com_is_online() && get_storage_flag() && measurement_range_valid(activeMeasurement.startTime,activeMeasurement.stopTime,epochNow_Ms))
		  { /* Process snapshot configuration and push parsed measurement block to ring buffer */
//#if PRINT_APP_STORAGE_DBG_MSG
//		    xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] Push measurement to ring buffer.\n", 0);
//#endif
			uint8_t  * pbuf     = (uint8_t*)temporaryBlock.buffer;
			uint32_t * pnelems  = (uint32_t*)&temporaryBlock.blockElems;
			uint32_t nelems 	= 0;
			temporaryBlock.cycleCounter = cycleCounter; /* Signal which cycle counter this block is related to */
			*pnelems = storage_meas_create_storage_data_block(cycleCounter, 0x80, pbuf); /* Build header of block, 0x80 = status of data set */
			config_createStoragePacket(&snapshotconf, (uint8_t*) pbuf+5, (unsigned int *) &nelems); /* Copy bytes in the buffer */
			*pnelems += nelems;
			mes_ring_buffer_queue(&measRingBuffer, temporaryBlock); /* Queue the temporaryBlock to ring buffer  */
		  }
		  else if(tablet_com_is_online() && get_storage_flag() && !(measurement_range_valid(activeMeasurement.startTime,activeMeasurement.stopTime,epochNow_Ms)))
		  { /* Range of measurement is not valid anymore while data storage is enabled:  stop storage after flushing ring buffer to SD card */
#if PRINT_APP_STORAGE_DBG_MSG
		    xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] Invalid measurement range, flush ring buffer and disable storage.\n", 0);
#endif
			set_storage_flag(false);											/* Disable the storage */
			app_sd_writer_notify();												/* Notify SD writer task to flush ring buffer and write it on SD card */
			storage_meas_close_file(&lHandler);									/* Close measurement file */
			serviceQueueMsg.status			= measurement_StatusOriginMeasurementList;
			serviceQueueMsg.action			= measurement_StopMeasurement;
			serviceQueueMsg.measurementId	= activeMeasurement.id;
			serviceQueueMsg.startTime		= activeMeasurement.startTime;
			serviceQueueMsg.stopTime		= activeMeasurement.stopTime;
			android_com_post(serviceQueueMsg); 									/* Notify tablet_com thread that measurement from the measurement list has to be stopped */
			memset(&serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t)); /* Put the service Queue Msg in a known state */
			activeMeasurement = get_active_measurement(&serviceQueueMsg);		/* Check for an active measurement in measurement file */
		  }
		  else /* An Error occur during the automated measurement */
		  { /* if storage flag is disabled, close the file. This is either because android device has been disconnected or watchdog message has not been sent correctly */
#if PRINT_APP_STORAGE_DBG_MSG
			xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] Error occurred during automated measurement, storage file will be closed.\n", 0);
#endif
			set_storage_flag(false); 											/* Disable the storage */
			storage_meas_close_file(&lHandler); 								/* Close the measurement file */
			memset(&serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t)); /* Put the serviceQueueMsg in a Known state */
		  }
		} /* end of: An automated measurement is requested from measurement list file */
		else if(serviceQueueMsg.status == measurement_StatusOriginAndroidDevice)
		{ /* A measurement request is initiated by Android device */
		  if(tablet_com_is_online() && get_storage_flag() && serviceQueueMsg.action == measurement_StartMeasurement)
		  { /* Measurement is running, push measurement to ring buffer */
//#if PRINT_APP_STORAGE_DBG_MSG
//		    xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] Push measurement to ring buffer.\n", 0);
//#endif
			uint8_t  * pbuf		= (uint8_t *)temporaryBlock.buffer;
			uint32_t * pnelems	= (uint32_t *)&temporaryBlock.blockElems;
			uint32_t nelems		= 0;
			temporaryBlock.cycleCounter = cycleCounter; /* Signal which cycle counter this block is related to */
			*pnelems = storage_meas_create_storage_data_block(cycleCounter, 0x80, pbuf); /* Build header of measurement block, 080 = status of data set */
			config_createStoragePacket(&snapshotconf, (uint8_t*) pbuf+5, (unsigned int *) &nelems); /* Copy bytes in the buffer */
			*pnelems += nelems;
			mes_ring_buffer_queue(&measRingBuffer, temporaryBlock); /* Now place the temporary block inside the ring buffer */
		  }
		  else if(tablet_com_is_online() && get_storage_flag() && serviceQueueMsg.action == measurement_StopMeasurement)
		  { /* Stop measurement request received while data storage is enabled:  stop storage after flushing ring buffer to SD card */
#if PRINT_APP_STORAGE_DBG_MSG
		    xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] Stop measurement received, flush ring buffer and disable storage.\n", 0);
#endif
			app_sd_writer_notify();												/* Flush ring buffer */
			set_storage_flag(false);											/* Disable storage */
			stomeas_header_t tempHeader;										/* Set header before closing file */
			tempHeader.setup_id			= decodedConfig.conf.setupID;
			tempHeader.version			= decodedConfig.conf.version;
			tempHeader.company_id		= decodedConfig.conf.companyID;
			tempHeader.measurement_id	= serviceQueueMsg.measurementId;
			tempHeader.start_time		= serviceQueueMsg.startTime;
			tempHeader.stop_time		= serviceQueueMsg.stopTime;
			tempHeader.block_length		= config_getStoragePacketSize(&decodedConfig.conf);
			storage_meas_set_header(&lHandler, &tempHeader);
			storage_meas_close_file(&lHandler);									/* Close measurement file */
			storage_meas_set_file_name(serviceQueueMsg.measurementId);			/* Set name of measurement file launched from android device */
			memset(&serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t)); /* Put the service QueueMsg in a known state */
		  }
		  else /* An error occurred during measurement because android device was disconnected */
		  { /* if storage flag is disabled, close the file. This is either because android device has been disconnected or watchdog message has not been sent correctly */
#if PRINT_APP_STORAGE_DBG_MSG
			xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_CYCLE_COUNTER] The Android device was disconnected.\n", 0);
#endif
			app_sd_writer_notify();												/* Flush ring buffer */
			set_storage_flag(false);											/* Disable storage  */
			stomeas_header_t tempHeader;										/* Set  header of file before closing it */
			tempHeader.company_id		= decodedConfig.conf.companyID;
			tempHeader.block_length		= config_getStoragePacketSize(&decodedConfig.conf);
			tempHeader.setup_id			= decodedConfig.conf.setupID;
			tempHeader.version			= decodedConfig.conf.version;
			tempHeader.measurement_id	= serviceQueueMsg.measurementId;
			tempHeader.start_time		= serviceQueueMsg.startTime;
			tempHeader.stop_time		= epochNow_Ms;
			// todo : set the measurment ID
			storage_meas_set_header(&lHandler, &tempHeader);
			storage_meas_close_file(&lHandler);									/* close file  */
			storage_meas_set_file_name(serviceQueueMsg.measurementId);			/* Set name of measurement file */
			memset(&serviceQueueMsg, 0, sizeof(measurementsServiceQueueMsg_t));	/* Put serviceQueueMsg in a known state */
		  }
		} /* End of: a measurement request is initiated by Android device */
		if(get_storage_flag())
		{
		  flushcounter++;
		  if (flushcounter == 25)
		  { /* Every 25 cycles, notify the sdwriter thread that ring buffer can be flushed and written to sd card */
		    app_sd_writer_notify();
			flushcounter = 0;
		  }
		}
	  } /* End of notification handler from syncThread (app_sync.c). */
	  if((ulNotificationValue & APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE) == APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE)
	  { /* Received notification from fc_measurement_list_data_rx_cplt_callback (app_tablet_com.c). */
		/* This happens when a new measurement list file is received and ready to be decoded. */
#if PRINT_APP_STORAGE_DBG_MSG
		xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE] Get a new measurement list file.\n", 0);
#endif
		measRes = measurement_list_read_file(); /* Read measurement list file and store data in measurement list file structure */
		if(measRes == meas_list_op_ok)
		{
		  if (measurementsList.status == measurement_list_correct)
		  {
			if(serviceQueueMsg.status == measurement_StatusOriginIDLE || serviceQueueMsg.status == measurement_StatusOriginAndroidDevice)
			{ /* Get active measurement from measurement list structure, create corresponding storage measurement file and notify tablet_com thread */
#if PRINT_APP_STORAGE_DBG_MSG
		      xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorageThread] [APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE] Decoding of measurement list done.\n", 0);
#endif
			  activeMeasurement = get_active_measurement(&serviceQueueMsg);
			}
		  }
		}
	  }
	}
  }
}

/**
 * @fn static void measStorWriteThread(const void * params)
 * @brief Task in charge of writing in the storage file
 * @param
 * @return
 */
static void measStorWriteThread(const void * params)
{
  sdWriterServiceQueueMsg_t rxMsg;
  memset(&rxMsg, 0, sizeof(sdWriterServiceQueueMsg_t));
  for(;;)
  {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
//#if PRINT_APP_STORAGE_DBG_MSG
//	xQueueSend(pPrintQueue, "[APP_STORAGE] [measStorWriteThread] Starting to flush the measurements to the ring buffer.\n", 0);
//#endif
    set_writer_flag(true);
	uint32_t t0write = HAL_GetTick();
	uint32_t nBlocks = mes_ring_buffer_num_items(&measRingBuffer);	/* Number of elements in the ring buffer */
	reset_bigBuffer_elems();										/* Reset the number of elements in the big buffer */
//#if PRINT_APP_STORAGE_DBG_MSG
//		sprintf(string, "%u [APP_STORAGE] [measStorWriteThread] Number of blocks in the ring buffer before: %u\n",(unsigned int) HAL_GetTick(), (unsigned int)nBlocks);
//		xQueueSend(pPrintQueue, string, 0);
//#endif
	if (nBlocks)
	{
	  for (uint32_t  i = 0; i<nBlocks; i++)
	  {
		if (mes_ring_buffer_dequeue(&measRingBuffer, &tempBlockFlush))
		{
		  memcpy(&bigBuffer[bigBufferElems], tempBlockFlush.buffer, tempBlockFlush.blockElems);
		  bigBufferElems += tempBlockFlush.blockElems;
		}
	  }
	  if (get_bigBuffer_elems())
	  {
//#if PRINT_APP_STORAGE_DBG_MSG
//	    sprintf(string, "%u [APP_STORAGE] [measStorWriteThread] Got to write %u elements.\n",(unsigned int) HAL_GetTick(), (unsigned int)bigBufferElems);
//		xQueueSend(pPrintQueue, string, 0);
//#endif
		t0write = HAL_GetTick();
		if (storage_meas_append_raw_data(&lHandler, bigBuffer, bigBufferElems) == STOMEAS_OP_OK)
		{
//#if PRINT_APP_STORAGE_DBG_MSG
//		  sprintf(string, "%u [APP_STORAGE] [measStorWriteThread] Write successful. It took %u ms.\n",(unsigned int) HAL_GetTick(), (unsigned int) (HAL_GetTick()-t0write));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
		}
		//TODO: test return code.
	  }
	}
	nBlocks = mes_ring_buffer_num_items(&measRingBuffer);
//#if PRINT_APP_STORAGE_DBG_MSG
//		sprintf(string, "%u [APP_STORAGE] [measStorWriteThread] Number of blocks in the ring buffer after: %u.\n",(unsigned int) HAL_GetTick(), (unsigned int)nBlocks);
//		xQueueSend(pPrintQueue, string, 0);
//#endif
	set_writer_flag(false);
  }
}

/**
 * @fn static uint32_t reset_bigBuffer_elems(void)
 * @brief Reset the number of elements in the big buffer
 * @param
 * @return uint32_t
 * Return the number of elements in the big buffer
 */
static uint32_t reset_bigBuffer_elems(void)
{
  bigBufferElems = 0;
  return bigBufferElems;
}

/**
 * @fn static uint32_t get_bigBuffer_elems(void)
 * @brief Return the number of elements in the big buffer
 * @param
 * @return uint32_t
 * Return the elements in the buffer
 */
static uint32_t get_bigBuffer_elems(void)
{
  return bigBufferElems;
}

/**
 * @fn static void set_writer_flag(void)
 * @brief Set the flag witer busy
 * @param
 * @return
 *
 */
static void set_writer_flag(bool value)
{
  flag_WriterBusy = value;
}

/**
 * @fn    : measurement_struct_t get_active_measurement(measurementsServiceQueueMsg_t * serviceQueueMsg)
 * @brief : check for an active measurement in measurement list file and return the reference ID, the start time and the stop time
 * @param : measurementsServiceQueueMsg_t * This is the message sent over the queue in charge of the communication between the storage thread and the thread in charge of the communication
 * @return: the active measurement structure = measurement_struct_t
 */
static measurement_struct_t get_active_measurement(measurementsServiceQueueMsg_t * serviceQueueMsg)
{
  measurement_struct_t activeMeasurement;
  memset(&activeMeasurement, 0, sizeof(measurement_struct_t));
  if(serviceQueueMsg == NULL || measurementsList.elems == 0 || !tablet_com_is_online())
  {
	return activeMeasurement;
  }
  uint64_t epochNow_Ms = 0;		/* Use to get the epoch time */
  bool exitCondition = false;	/* Boolean value used for the exist condition to stop searching for an active measurement*/
  char nameStoMeasFile[32];		/* Used for the name of the file */
  int sizeOfParsedName = 0;
  memset(nameStoMeasFile, 0x00, 32);
  for(uint32_t index = 0; index < measurementsList.elems; index++)
  { /* Scroll the measurement list until we get an active measurement */
#if PRINT_APP_STORAGE_DBG_MSG
	xQueueSend(pPrintQueue, "[APP_STORAGE] [get_active_measurement] Scroll through measurement list...\n", 0);
#endif
	uint64_t startTime = measurementsList.list[index].startTime;	/* Temporary variables for ease of reading */
	uint64_t stopTime  = measurementsList.list[index].stopTime;		/* Temporary variables for ease of reading */
	app_rtc_get_unix_epoch_ms(&epochNow_Ms);						/* Get current time as a Unix Epoch in ms */
	if (measurement_range_valid(startTime, stopTime, epochNow_Ms))
	{ /* Retrieved measurement is valid */
#if PRINT_APP_STORAGE_DBG_MSG
	  xQueueSend(pPrintQueue, "[APP_STORAGE] [get_active_measurement] Found valid measurement...\n", 0);
#endif
	  if(serviceQueueMsg->status == measurement_StatusOriginAndroidDevice)
	  { /* Measurement from android device was ongoing... close file first */
		stomeas_header_t tempHeader;
		tempHeader.company_id 		= decodedConfig.conf.companyID;
		tempHeader.block_length 	= config_getStoragePacketSize(&decodedConfig.conf);
		tempHeader.setup_id 		= decodedConfig.conf.setupID;
		tempHeader.version 			= decodedConfig.conf.version;
		tempHeader.measurement_id 	= serviceQueueMsg->measurementId;
		tempHeader.start_time 		= serviceQueueMsg->startTime;
		tempHeader.stop_time 		= epochNow_Ms;
		storage_meas_set_header(&lHandler, &tempHeader);
		storage_meas_close_file(&lHandler);
		/*Send a notification to the table_com thread to stop the measurement from the android device */
		//todo
	  }
	  activeMeasurement = measurementsList.list[index]; /* Copy this measurement in activeMeasurement */
	  sizeOfParsedName  = storage_meas_parse_file_name(nameStoMeasFile, 32, activeMeasurement.id); /* Parse name of storage file */
	  stomeas_res_t res = storage_meas_check_file_exists(nameStoMeasFile); /* Check if the measurement data storage file exists */
	  switch (res)
	  {
	    case STOMEAS_OP_FILE_EXISTS:
		{
#if PRINT_APP_STORAGE_DBG_MSG
		  xQueueSend(pPrintQueue, "[APP_STORAGE] [get_active_measurement] [STOMEAS_OP_FILE_EXISTS] File does exist, checking header...\n", 0);
#endif
		  stomeas_res_t stores = storage_meas_set_file(&lHandler, nameStoMeasFile, sizeOfParsedName); /* Open file */
		  if (storage_meas_get_header(&lHandler, &lHeader) != STOMEAS_OP_OK) /* Error: measurement's file exist for this ID, but header is incorrect */
		  { /* Can happen if the configuration on the main board does not match the configuration for this measurement */
#if PRINT_APP_STORAGE_DBG_MSG
			sprintf(string, "[APP_STORAGE] [get_active_measurement] [STOMEAS_OP_FILE_EXISTS] FATAL_ERROR: Measurement file exist for ID=%u, but the header inside the file is incorrect.\n",(unsigned int) activeMeasurement.id);
			xQueueSend(pPrintQueue, string, 0);
#endif
			memset(&activeMeasurement, 0, sizeof(measurement_struct_t)); /* Put active measurement in a known state */
		  }
		  else
		  {
#if PRINT_APP_STORAGE_DBG_MSG
			sprintf(string, "[APP_STORAGE] [get_active_measurement] [STOMEAS_OP_FILE_EXISTS] [STOMEAS_OP_OK] Get an active measurement, Active Measurement ID : %u, Start time: %lu, Stop time: %lu.\n",
							(unsigned int) activeMeasurement.id,(long unsigned int) activeMeasurement.startTime,(long unsigned int) activeMeasurement.stopTime);
			xQueueSend(pPrintQueue, string, 0);
#endif
			app_rtc_calculate_cycle_counter_from_reference(activeMeasurement.startTime); /* Calculate new cycle counter in relation to start time defined in measurement list */
			if(tablet_com_is_online())
			{ /* If android device is connected enable storage flag */
			  set_storage_flag(true);
			}
			serviceQueueMsg->status = measurement_StatusOriginMeasurementList;	/* Origin of measurement comes from measurement list file */
			serviceQueueMsg->action = measurement_StartMeasurement;				/* Set corresponding action of measurement  */
			serviceQueueMsg->measurementId = activeMeasurement.id;				/* Set ID of measurement */
			serviceQueueMsg->startTime = activeMeasurement.startTime;			/* Set start time of measurement */
			serviceQueueMsg->stopTime = activeMeasurement.stopTime;				/* Set stop time of measurement */
			android_com_post(*serviceQueueMsg);									/* Send notification to the uplinkManager thread */
			exitCondition = 1;													/* Exit for loop */
		  }
		  break;
		}
		case STOMEAS_OP_FILE_DOES_NOT_EXIST:
		{
#if PRINT_APP_STORAGE_DBG_MSG
		  sprintf(string, "%u [APP_STORAGE] [get_active_measurement] [STOMEAS_OP_FILE_DOES_NOT_EXIST] The measurement File does not exist for this measurement ID : %u.\n",(unsigned int) HAL_GetTick(),(unsigned int) measurementsList.list[index].id);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  stomeas_res_t stores = storage_meas_set_file( &lHandler, nameStoMeasFile, sizeOfParsedName); /* Create new storage file */
		  if(stores == STOMEAS_OP_OK)
		  { /* Set the header of the storage file */
			stomeas_header_t tempHeader;
			tempHeader.company_id		= decodedConfig.conf.companyID;
			tempHeader.block_length		= config_getStoragePacketSize(&decodedConfig.conf);
			tempHeader.setup_id			= decodedConfig.conf.setupID;
			tempHeader.version			= decodedConfig.conf.version;
			tempHeader.measurement_id	= activeMeasurement.id;
			tempHeader.start_time		= activeMeasurement.startTime;
			tempHeader.stop_time		= activeMeasurement.stopTime;
			stores = storage_meas_set_header(&lHandler, &tempHeader);
			if(stores == STOMEAS_OP_OK)
			{
			  app_rtc_calculate_cycle_counter_from_reference(activeMeasurement.startTime); /* Calculate new cycle counter, depending on startTime */
			  if(tablet_com_is_online())
			  { /* If android device is connected enable storage flag */
			    set_storage_flag(true);
			  }
#if PRINT_APP_STORAGE_DBG_MSG
			  sprintf(string, "%u [APP_STORAGE] [get_active_measurement] [STOMEAS_OP_FILE_DOES_NOT_EXIST] [STOMEAS_OP_OK] Get an active measurement, Active Measurement ID : %u, Start time: %lu, Stop time: %lu.\n",
							(unsigned int) HAL_GetTick(),(unsigned int) activeMeasurement.id,(long unsigned int) activeMeasurement.startTime,(long unsigned int) activeMeasurement.stopTime);
			  xQueueSend(pPrintQueue, string, 0);
#endif
			  serviceQueueMsg->status 			= measurement_StatusOriginMeasurementList;	/* Origin of measurement comes from measurement list file */
			  serviceQueueMsg->action 			= measurement_StartMeasurement;				/* Set corresponding action */
			  serviceQueueMsg->measurementId 	= activeMeasurement.id;						/* Set ID of measurement */
			  serviceQueueMsg->startTime 		= activeMeasurement.startTime;				/* Set start time of measurement */
			  serviceQueueMsg->stopTime 		= activeMeasurement.stopTime;				/* Set stop time of measurement */
			  android_com_post(*serviceQueueMsg);											/* Send notification to tablet_com task */
			  exitCondition = 1;															/* Exit for loop */
			}
		  }
		  break;
		}
		default:
		{
#if PRINT_APP_STORAGE_DBG_MSG
		  xQueueSend(pPrintQueue, "[APP_STORAGE] [get_active_measurement] [default] Unhandled error when checking if file exists.\n", 0);
#endif
		}
	  } /* end of switch (res) */
	} /* end of retrieved measurement is valid */
	if (exitCondition)
	{
	  break; /* Exit for loop as a correct measurement is retrieved */
	}
  } /* end of for-loop or scrolling the measurement list until we get an active measurement */
  return activeMeasurement;
}

/**
 * @fn static void app_sd_writer_notify(uint32_t notValue)
 * @brief Give a notification to the SD writer thread
 * @param
 * @return
 */
static void app_sd_writer_notify()
{
  if (sdWriterThreadHandler != NULL)
  {
	xTaskNotifyGive(sdWriterThreadHandler);
  }
}

/**
 *@fn void app_storage_notify(uint32_t notValue)
 *@brief Give a notification to the measurement storage task
 *@param
 *@return
 */
void app_storage_notify(uint32_t notValue)
{
  if (measStorageThreadHandler != NULL)
  {
    xTaskNotify( measStorageThreadHandler, notValue, eSetBits);
  }
}

/**
 *@fn void app_storage_notify_from_isr(uint32_t notValue)
 *@brief Give a notification to the measurement storage task from an ISR
 *@param
 *@return
 */
void app_storage_notify_from_isr(uint32_t notValue)
{
  BaseType_t xHigherPriorityTaskWoken;
  xHigherPriorityTaskWoken = pdFALSE;
  if (measStorageThreadHandler != NULL)
  {
	xTaskNotifyFromISR( measStorageThreadHandler, notValue, eSetBits, &xHigherPriorityTaskWoken );
	portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
  }
}

/**
 * @fn void app_storage_prepare_for_shutdown(void)
 * @brief
 * @param
 * @return
 */
void app_storage_prepare_for_shutdown(void)
{
  storage_meas_close_file(&lHandler);
}

/**
 * @fn int app_storage_active(void)
 * @brief Return the state of the data recording
 * The private function static void get_storage_flag(void) as the same purpose
 * @param
 * @return
 * @todo Replace by the get_storage_flag_function()
 */
int app_storage_active(void)
{
  return storageEnabled;
}

/**
 * @fn static void set_storage_flag(bool value)
 * @brief set the storage flag to value
 * @param bool value
 * boolean value which storage flag will be set
 * @return
 */
static void set_storage_flag(bool value)
{
  storageEnabled = value;
}

/**
 * @fn static bool get_storage_flag(void)
 * @brief Get the storage flag
 * @param
 * @return
 */
static bool get_storage_flag(void)
{
  return storageEnabled;
}

/**
 * @fn static bool measurement_range_valid(uint64_t startTime, uint64_t stopTime, uint64_t now)
 * @brief Check if the measurement time interval is valid
 * @param uint64_t startTime, uint64_t stopTime, uint64_t now
 * Take the start time the stop time and the actual time to check if the range is valid
 * @return
 */
static bool measurement_range_valid(uint64_t startTime, uint64_t stopTime, uint64_t now)
{
  bool isValid = ((startTime <= now) && (now <= stopTime));
  return isValid;
}

static int app_storage_open_new_file(uint32_t measurementId)
{
  char name[32];
  int cx = 0;
  memset(name, 0, 32);
  cx = snprintf(name, 32, "m%06u.BIN", (unsigned int) measurementId);	/* Formatting the name of the file */
#if PRINT_APP_STORAGE_DBG_MSG
  sprintf(string, "%u [APP_STORAGE] [app_storage_open_new_file] Opening new file: %s.\n", name);
  xQueueSend(pPrintQueue, string, 0);
#endif
  storage_meas_close_file(&lHandler);									/* Make sure of that the previous file was closed */
  stomeas_res_t stores = storage_meas_set_file(&lHandler, name, cx );	/* Open the file and set the name of the file  */
  return stores;
}
