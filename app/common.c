/**
 * @file common.c
 * @brief Contain ressources used by all the applications.
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
#include <string.h>

#include "common.h"
#include "usart.h"  //to declare huart5
#include "app_init.h" // to declare QueueHandle_t

RAM1_PLACE protectedRawConfig_t 		rawConfig;
RAM1_PLACE protectedDecodedConfig_t 	decodedConfig;
RAM1_PLACE decoded_config_t 			snapshotconf;

#define COMMON_DBG_PRINTF 1
extern char string[];
extern QueueHandle_t pPrintQueue;

/* Function to get the access to the protected data container.
 * @param nothing
 * @return 1 in case of success, 0 in case of failure
 */
static unsigned int getRawConfigAccess()
{
  if( !rawConfig.mutex )
  {
	/* The mutex is incorrect */
	return 0;
  }
  else
  {
	if (xSemaphoreTake(rawConfig.mutex, ( TickType_t ) 10) == pdTRUE)
	{
	  /* Operation success */
//#if COMMON_DBG_PRINTF
//      sprintf(string, "[COMMON] [getRawConfigAccess] xSemaphoreTake(rawConfig.mutex, ( TickType_t ) 10) success.\n");
//		xQueueSend(pPrintQueue, string, 0);
//#endif
	  return 1;
	}
	else
	{
	  /* Failed to retrieve the mutex after 10 Tick */
//#if COMMON_DBG_PRINTF
//      sprintf(string, "[COMMON] [getRawConfigAccess] xSemaphoreTake(rawConfig.mutex, ( TickType_t ) 10) failed.\n");
//		xQueueSend(pPrintQueue, string, 0);
//#endif
	  return 0;
	}
  }
}

/* Function to release the access to the protected data container.
 * @param nothing
 * @return nothing
 */
static int releaseRawConfigAccess()
{
  if( !rawConfig.mutex )
  {
	/* The mutex is incorrect */
//#if COMMON_DBG_PRINTF
//	sprintf(string, "[COMMON] [releaseRawConfigAccess The mutex is incorrect.\n");
//	xQueueSend(pPrintQueue, string, 0);
//#endif
	return 0;
  }
  else
  {
	xSemaphoreGive(rawConfig.mutex );
//#if COMMON_DBG_PRINTF
//	sprintf(string, "[COMMON] [releaseRawConfigAccess] xSemaphoreGive(rawConfig.mutex) done.\n");
//	xQueueSend(pPrintQueue, string, 0);
//#endif
	return 1;
  }
}

/* Function to get the access to the protected data container.
 * @param nothing
 * @return 1 in case of success, 0 in case of failure
 */
static unsigned int getDecodedConfigAccess()
{
  if( !rawConfig.mutex )
  {
	/* The mutex is incorrect */
	return 0;
  }
  else
  {
    if (xSemaphoreTake(decodedConfig.mutex, ( TickType_t ) 10) == pdTRUE)
    {
      /* Operation success */
//#if COMMON_DBG_PRINTF
//      sprintf(string, "[COMMON] [getDecodedConfigAccess] xSemaphoreTake(decodedConfig.mutex, (TickType_t) 10) success.\n");
//    	xQueueSend(pPrintQueue, string, 0);
//#endif
	  return 1;
	}
	else
	{
	  /* Failed to retrieve the mutex after 10 Tick */
//#if COMMON_DBG_PRINTF
//      sprintf(string, "[COMMON] [getDecodedConfigAccess] xSemaphoreTake(decodedConfig.mutex, (TickType_t) 10) failed.\n");
//		xQueueSend(pPrintQueue, string, 0);
//#endif
	  return 0;
	}
  }
}

/* Function to release the access to the protected data container.
 * @param nothing
 * @return nothing
 */
static void releaseDecodedConfigAccess()
{

  if( !rawConfig.mutex )
  {
	/* The mutex is incorrect */
#if COMMON_DBG_PRINTF
	sprintf(string, "[COMMON] [releaseDecodedConfigAccess] The mutex is incorrect.\n");
	xQueueSend(pPrintQueue, string, 0);
#endif
  }
  else
  {
	xSemaphoreGive(decodedConfig.mutex );
  }
}


void common_config_init()
{
/* Create the mutex that protects the container */
  rawConfig.mutex   = xSemaphoreCreateMutex();
  /* Set the function pointers */
  rawConfig.get     = getRawConfigAccess;
  rawConfig.release = releaseRawConfigAccess;
  /* Create the mutex that protects the container */
  decodedConfig.mutex   = xSemaphoreCreateMutex();
  decodedConfig.get     = getDecodedConfigAccess;
  decodedConfig.release = releaseDecodedConfigAccess;
  /* Set the default value of the ID */
  decodedConfig.conf.setupID = 0;
  decodedConfig.conf.version = 0;
  decodedConfig.conf.companyID = 0;
  decodedConfig.conf.numberOfInstruments = 0;
  decodedConfig.conf.cycleCounter = 0;
  decodedConfig.state = CONF_NOT_READY;
}

/* Return the current  */
unsigned int getConfigId()
{
unsigned int id = 0;
//  decodedConfig.get();	/* Get access */
  id = decodedConfig.conf.setupID;
//  decodedConfig.release(); /*Release access */
  return id;
}

unsigned int getConfigVersion()
{
  unsigned int version = 0;
//  decodedConfig.get();	/* Get access */
  version = decodedConfig.conf.version;
//  decodedConfig.release(); /*Release access */
  return version;
}

/*
typedef enum {
	app_imu,
	app_gps,
	app_dx,
	app_pg,
	app_linx,
	app_can_poller

} app_type_t;
*/

/* This list all the possible nodes and their related function (that must be implemented!)*/
/* a node is described by:
 * - its name
 * - a decoding function (buffer to datatype)
 * - binary parsing function (for export)
 * - ascii parsing function (printf, datalog)
 */
node_decriptor nodes[] = {
  {SETUP_PRM_COMM_METHOD_IMU,                      "internal imu",       0, NULL, NULL, NULL,  20, sizeof(imu_data_t)},
  {SETUP_PRM_COMM_METHOD_IMU,                      "external imu",       1, NULL, NULL, NULL,  20, sizeof(imu_data_t)},
  {SETUP_PRM_COMM_METHOD_GPS,                      "internal gps",       0, NULL, NULL, NULL, 500, sizeof(gps_data_t)},
  {SETUP_PRM_COMM_METHOD_JOYSTICK_PENNY_GILES,     "pg interface",       0, NULL, NULL, NULL, 500, sizeof(pwc_data_t)},
  {SETUP_PRM_COMM_METHOD_JOYSTICK_DYNAMIC_CONTROL, "dx2 interface",      0, NULL, NULL, NULL, 500, sizeof(pwc_data_t)},
  {SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR,      "can dist sensor d1", 1, NULL, NULL, NULL, 100, sizeof(can_distance_node_d1)},
  {SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR,      "can dist sensor d2", 2, NULL, NULL, NULL, 100, sizeof(can_distance_node_d2)},
  {SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR,      "can dist sensor d3", 3, NULL, NULL, NULL, 100, sizeof(can_distance_node_d3)},
  {SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR,      "can dist sensor d4", 4, NULL, NULL, NULL, 100, sizeof(can_distance_node_d4)},
  {SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR,      "can dist sensor d5", 5, NULL, NULL, NULL, 100, sizeof(can_distance_node_d5)}
};

parsing_assoc_t parsingList[MAX_ASSOCIATIONS] = {
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_NONE, 			   NULL,                                  NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT,   datatypeToRaw_joystick_dx2,            datatypeToAscii_joystick_dx2,      NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT,    datatypeToRaw_joystick_pg,             datatypeToAscii_joystick_pg,       NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT,  datatypeToRaw_joystick_linx,           NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC, 	   datatypeToRaw_imu_9axis_rot_vec,       datatypeToAscii_imu_9axis_rot_vec, NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS, 		   datatypeToRaw_imu_6axis,               datatypeToAscii_imu_6axis,         NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA, 		   datatypeToRaw_gpsMinData,              datatypeToAscii_gpsMinData,        NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS, 		   NULL,                                  NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS, 	   NULL,                                  NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1, datatypeToRaw_canDistanceNodeD1,       datatypeToAscii_canDistanceNodeD1, rawToStruct_canDistanceNodeD1},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2, datatypeToRaw_canDistanceNodeD2,       datatypeToAscii_canDistanceNodeD2, rawToStruct_canDistanceNodeD2},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3, datatypeToRaw_canDistanceNodeD3,       datatypeToAscii_canDistanceNodeD3, rawToStruct_canDistanceNodeD3},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4, datatypeToRaw_canDistanceNodeD4,       datatypeToAscii_canDistanceNodeD4, rawToStruct_canDistanceNodeD4},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5, datatypeToRaw_canDistanceNodeD5,       datatypeToAscii_canDistanceNodeD5, rawToStruct_canDistanceNodeD5},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D6, datatypeToRaw_canDistanceNodeD6,       datatypeToAscii_canDistanceNodeD6, rawToStruct_canDistanceNodeD6},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D7, datatypeToRaw_canDistanceNodeD7,       NULL,                              rawToStruct_canDistanceNodeD7},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D8, datatypeToRaw_canDistanceNodeD8,       NULL,                              rawToStruct_canDistanceNodeD8},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_RTC,                 datatypeToRaw_RTC,                     NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD,               datatypeToRaw_usbad_instrument,        NULL,                              NULL},
  {SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD_SENSOR,        datatypeToRaw_usbad_sensor_instrument, NULL,                              NULL}
  /* add more */
};
