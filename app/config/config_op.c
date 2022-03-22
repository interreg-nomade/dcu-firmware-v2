/**
 * @file config_op.c
 * @brief Contains all the operations related to the configuration
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
#include "config_op.h"
#include "nodes.h"
#include "parsing.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "usart.h"  //to declare huart5
#include "../app_init.h" // to include Queuehandle_t

#define CYCLE_COUNTER_SIZE 					4
#define MEASUREMENT_DATASET_STATUS_SIZE		1
#define MEASUREMENT_DATA_STATUS_SIZE		1
#define INSTRUMENT_ID_SIZE					2

#define MEASUREMENT_DATA_OK					0x80
#define MEASUREMENT_DATA_NOK				0xFE

#define assert( x ) if( ( x ) == 0 ) {return 0;}

#define CONFIG_LINK_PRINT_INFOS 		0//1
#define CONFIG_DATA_ALLOC_PRINT_INFOS 	0//1
#define CONFIG_OP_DBG_PRINT 			0//1


extern char string[];
extern QueueHandle_t pPrintQueue;

int config_linkParsingFunctions(decoded_config_t * pConf)
{
  for (unsigned int i = 0; i<pConf->numberOfInstruments; i++)
  {
    if (pConf->instruments)
	{
	  switch (pConf->instruments[i].dataTypeOutput)
	  {
	    case SETUP_PRM_DATA_OUTPUT_DATATYPE_NONE:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter data output data type none, assigning: NULL.\n", 0);
#endif
  		  /* Pass */
		  break;
		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT, assigning: datatypeToRaw_joystick_dx2() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary =  datatypeToRaw_joystick_dx2;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT, assigning: datatypeToRaw_joystick_pg() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_joystick_pg;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT, no parsing function assigned.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  /* Not handled for the moment */
//		  pConf->instruments[i].parseBinary = NULL;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC, assigning: datatypeToRaw_imu_9axis_rot_vec() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_9axis_rot_vec;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS, assigning: datatypeToRaw_imu_6axis() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_6axis;
//		  break;
//		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUATBAT, assigning parsing function.\n", 0);
#endif
		  //pConf->instruments[i].parseBinary = datatypeToRaw_imu_quatBat;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT_GYRO_ACC, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat_gyro_acc;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT_GYRO_ACC_100HZ, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat_gyro_acc_100Hz;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_100HZ:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT_QUAT_100HZ, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat_100Hz;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT_QUAT_9DOF, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat_9DOF;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF_100HZ:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMUQUAT_QUAT_9DOF_100HZ, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_quat_9DOF_100Hz;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMU_GYRO_ACC_MAG, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_gyro_acc_mag;
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG_100HZ:
		{
#if CONFIG_LINK_PRINT_INFOS
		  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter IMU_GYRO_ACC_MAG_100HZ, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_imu_gyro_acc_mag_100Hz;
		  break;
		}

//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA, assigning: datatypeToRaw_gpsMinData() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_gpsMinData;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS, assigning: NULL.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = NULL;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS, assigning: NULL.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = NULL;
//		  break;
//		}
//		/* EDUCAT DISTANCE NODES PART BELOW */
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1, assigning: datatypeToRaw_canDistanceNodeD1() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_canDistanceNodeD1;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2, assigning: datatypeToRaw_canDistanceNodeD2() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_canDistanceNodeD2;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3, assigning: datatypeToRaw_canDistanceNodeD3() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_canDistanceNodeD3;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4, assigning: datatypeToRaw_canDistanceNodeD4() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_canDistanceNodeD4;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5, assigning: datatypeToRaw_canDistanceNodeD5() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_canDistanceNodeD5;
//		  break;
//		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_RTC:
		{
#if CONFIG_LINK_PRINT_INFOS
			  xQueueSend(pPrintQueue, "[config_op] [config_linkParsingFunctions] Found parameter RTC, assigning parsing function.\n", 0);
#endif
		  pConf->instruments[i].parseBinary = datatypeToRaw_RTC;
		  break;
		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD, assigning: datatypeToRaw_usbad_instrument() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_usbad_instrument;
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD_SENSOR:
//		{
//#if CONFIG_LINK_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD, assigning: datatypeToRaw_usbad_sensor_instrument() parsing function.\n",(unsigned int) HAL_GetTick());
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].parseBinary = datatypeToRaw_usbad_sensor_instrument;
//		  break;
//		}
		default:
		{
#if CONFIG_LINK_PRINT_INFOS
		  sprintf(string, "[config_op] [config_linkParsingFunctions] Found parameter %08X: unknown value.\n",pConf->instruments[i].dataTypeOutput);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		pConf->instruments[i].parseBinary = NULL;
		break;
		}
	  }
    }
  }
#if CONFIG_LINK_PRINT_INFOS
  sprintf(string, "%u [config_op] [config_linkParsingFunctions] Number of instruments done: %d.\n",(unsigned int) HAL_GetTick(),pConf->numberOfInstruments);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return pConf->numberOfInstruments;
}

int config_linkParsingFunctions_fromList(decoded_config_t * pConf, parsing_assoc_t * list, unsigned int nNodes)
{
  for (unsigned int i = 0; i<pConf->numberOfInstruments; i++)
  {
    if (pConf->instruments)
    {
	  for (unsigned int j = 0; j < nNodes; j++)
	  {
	    if (list[j].dataOutputType == pConf->instruments[i].dataTypeOutput)
		{
#if CONFIG_LINK_PRINT_INFOS
		  sprintf(string, "%u [config_op] [config_linkParsingFunctions_fromList] Found functions for instrument %d to parse the data type output of the instrument.\n",(unsigned int) HAL_GetTick(),i);
		  xQueueSend(pPrintQueue, string, 0);
#endif
	      /* found functions to parse the datatype output of the instrument */
		  /* now linking it */
		  pConf->instruments[i].parseBinary = list[j].parsingFunctionBinary;
		  pConf->instruments[i].parseAscii  = list[j].parsingFunctionASCII;
		  pConf->instruments[i].decodeData  = list[j].parsingFunctionstruct;
		  break;
		}
	  }
	}
  }
#if CONFIG_LINK_PRINT_INFOS
  sprintf(string, "%u [config_op] [config_linkParsingFunctions_fromList] Number of instruments done: %d.\n",(unsigned int) HAL_GetTick(),pConf->numberOfInstruments);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return pConf->numberOfInstruments;
}

int config_allocateDataSpace(decoded_config_t * pConf)
{
  int module_id = 0; // to distinguish the different possible modules with the same definition
  for (unsigned int i = 0; i<pConf->numberOfInstruments; i++)
  {
    if (pConf->instruments)
	{
      switch (pConf->instruments[i].dataTypeOutput)
	  {
	    case SETUP_PRM_DATA_OUTPUT_DATATYPE_NONE:
	    {
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter data output data type none, not allocating memory.\n",(unsigned int) HAL_GetTick());
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = NULL;
		  /* Pass */
		  break;
		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKDX2OUTPUT, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(pwc_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(pwc_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(pwc_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKPGOUTPUT, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(pwc_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(pwc_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(pwc_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_JOYSTICKLINXOUTPUT, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(pwc_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(pwc_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(pwc_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(imu_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(imu_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(imu_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU6AXIS, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(imu_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(imu_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(imu_data_t));
//		  break;
//		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUATBAT, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT_GYRO_ACC, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT_GYRO_ACC_100HZ, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_100HZ:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT_100HZ, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT_9DOF, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF_100HZ:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUQUAT_9DOF_100HZ, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUGYRO_ACC_MAG, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG_100HZ:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter IMUGYRO_ACC_MAG_100HZ, allocating %d bytes.\n", sizeof(imu_100Hz_data_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(imu_100Hz_data_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(imu_100Hz_data_t));
		  break;
		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSMINDATA, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(gps_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(gps_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(gps_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSSTATUS, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(gps_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(gps_data_t));
//		  if (!pConf->instruments[i].data)
//	 	  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(gps_data_t));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_GPSDATASTATUS, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(gps_data_t));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(gps_data_t));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(gps_data_t));
//		  break;
//		}
//		/********** EDUCAT DISTANCE NODES PART BELOW .. */
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D1, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d1));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d1));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d1));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D2, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d2));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d2));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d2));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D3, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d3));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d3));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d3));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D4, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d4));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d4));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d4));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D5, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d5));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d5));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d5));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D6:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D6, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d6));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d6));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d6));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D7:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D7, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d7));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d7));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d7));
//		  break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D8:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_CANDISTANCENODES_D8, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(can_distance_node_d7));
//		  xQueueSend(pPrintQueue, string, 0);
//#endif
//		  pConf->instruments[i].data = malloc (sizeof(can_distance_node_d7));
//		  if (!pConf->instruments[i].data)
//		  {
//		    /* Pointer is still NULL */
//		  }
//		  memset(pConf->instruments[i].data, 0, sizeof(can_distance_node_d7));
//		  break;
//		}
		case SETUP_PRM_DATA_OUTPUT_DATATYPE_RTC:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "[config_op] [config_allocateDataSpace] Found parameter RTC, allocating %d bytes.\n", sizeof(uint64_t));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = malloc (sizeof(uint64_t));
		  if (!pConf->instruments[i].data)
		  {
		    /* Pointer is still NULL */
		  }
		  memset(pConf->instruments[i].data, 0, sizeof(uint64_t));
		  break;
		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//			sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(usbad_data_t));
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			pConf->instruments[i].data = malloc (sizeof(usbad_data_t));
//			if (!pConf->instruments[i].data)
//			{
//				/* Pointer is still NULL */
//			}
//			memset(pConf->instruments[i].data, 0, sizeof(usbad_data_t));
//			break;
//		}
//		case SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD_SENSOR:
//		{
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//			sprintf(string, "%u [config_op] [config_allocateDataSpace] Found parameter SETUP_PRM_DATA_OUTPUT_DATATYPE_USBAD_SENSOR, allocating %d bytes.\n",(unsigned int) HAL_GetTick(), sizeof(usbad_data_t));
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			pConf->instruments[i].data = malloc (sizeof(usbad_data_t));
//			if (!pConf->instruments[i].data)
//			{
//				/* Pointer is still NULL */
//			}
//			memset(pConf->instruments[i].data, 0, sizeof(usbad_data_t));
//			break;
//		}
		default:
		{
#if CONFIG_DATA_ALLOC_PRINT_INFOS
		  sprintf(string, "%u [config_op] [config_allocateDataSpace] Found unknown Data Type Output Value: %08X.\n",(unsigned int) HAL_GetTick(), sizeof(pConf->instruments[i].dataTypeOutput));
		  xQueueSend(pPrintQueue, string, 0);
#endif
		  pConf->instruments[i].data = NULL;
		  break;
		}
	  }
	}
  }
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//  sprintf(string, "%u [config_op] [config_allocateDataSpace] Number of instruments done: %d.\n",(unsigned int) HAL_GetTick(),pConf->numberOfInstruments);
//  xQueueSend(pPrintQueue, string, 0);
//#endif
  return pConf->numberOfInstruments;
}

int config_dbg_ascii(decoded_config_t * pConf)
{
  if ((!pConf))
  {
	/* One of the pointers is incorrect */
	return 0;
  }
  unsigned char bf[128];
  instrument_config_t * pInst;
  pInst = pConf->instruments;
  for (unsigned int i = 0; i < pConf->numberOfInstruments; i++)
  {
    if (pInst)
    {
      /* instrument pointer is correct */
      if (pInst->parseAscii)
      {
        /* Call the parsing function */
        unsigned int nbytes = 0;
        bf[0] = 0;
        nbytes = pInst->parseAscii(50, pInst->data, bf, 1);
#if CONFIG_DATA_ALLOC_PRINT_INFOS
        sprintf(string, "%u [config_op] [config_dbg_ascii] Parsing instrument ID=%d, bytes of data parsed = %d.\n",(unsigned int) HAL_GetTick(), pInst->instrumentID, nbytes);
		xQueueSend(pPrintQueue, string, 0);
#endif
        if (nbytes)
        {
#if CONFIG_DATA_ALLOC_PRINT_INFOS
          sprintf(string, "%u [config_op] [config_dbg_ascii] %s?\n",(unsigned int) HAL_GetTick(),bf);
          xQueueSend(pPrintQueue, string, 0);
#endif
        }
      }
    }
    else
    {
      // pointer @ null
    }
    pInst++;
  }
  return 1;
}
/**
 * @fn int config_createStreamPacket(decoded_config_t * pConf, unsigned char * pDest, unsigned int pSize)
 * @brief create the streaming packet to send to the android device
 * @param pConf pointer to the decoded configuration
 * @param pDest pointer to the destination buffer
 * @param pSize pointer to an unsigned int -> number of bytes written in the pDest buffer
 */
int config_createStreamPacket(decoded_config_t * pConf, unsigned char * pDest, unsigned int * pSize)
{
  assert(pConf != NULL);
  assert(pDest != NULL);
  assert(pSize != NULL);
  /* intermediate pointer used */
  instrument_config_t * pInst;
  /* Point to the decodedconfig instruments struct part */
  pInst = pConf->instruments;
  /* Initialize the number of bytes to be written to 0 */
  *pSize = 0;
  for (unsigned int i = 0; i < pConf->numberOfInstruments; i++)
  {
	if (pInst != NULL)
	{
	  /* Pointer of the instrument is ok */
	  /* instrument pointer is correct */
	  if (pInst->parseBinary)
	  {
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//      sprintf(string, "%u [config_op] [config_createStreamPacket] Parsing measure data of instrument ID %d.\n",(unsigned int) HAL_GetTick(), pInst->instrumentID);
//      xQueueSend(pPrintQueue, string, 0);
//#endif
		unsigned int nbytes = 0;
        /* Call the parsing function
		 * --> return number of bytes for each instruments */
		nbytes = pInst->parseBinary(pInst->data, pDest);
//#if CONFIG_DATA_ALLOC_PRINT_INFOS
//        sprintf(string, "%u [config_op] [config_createStreamPacket] %d bytes of data parsed.\n",(unsigned int) HAL_GetTick(), nbytes);
//        HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
		if (nbytes)
		{
		  pDest[nbytes++] = MEASUREMENT_DATA_OK;
		  pDest          += nbytes;
		  *pSize         += nbytes;
		}
	  }
	}
	else
	{
	  // pointer @ null
	}
	pInst++;
  }
  return 1;
}

/**
 * @fn config_createStoragePacket(decoded_config_t * pConf, unsigned char * dest, unsigned int * n)
 * @brief Create the storage packet which will be write in the SD card
 * @param decoded_config_t * pConf Pointer to the decoded configuration which handle the data of the different instruments
 * @param unsigned char * dest Destination pointer where the parsed data which will be write in the SD card will be store
 * @param unsigned int * n Size of the dest output buffer
 * @return int Error code
 */
int config_createStoragePacket(decoded_config_t * pConf,
		unsigned char * pDest,
		unsigned int * pSize)
{
	assert(pConf != NULL);
	assert(pDest != NULL);
	assert(pSize != NULL);

	instrument_config_t * pInst;

	*pSize = 0;

	pInst = pConf->instruments;

//#if CONFIG_OP_DBG_PRINT
//    sprintf(string, "%u [config_op] [config_createStoragePacket] Started.\n",(unsigned int) HAL_GetTick());
//    HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif

	for (unsigned int i = 0; i < pConf->numberOfInstruments; i++)
	{
		if (pInst != NULL)
		{
			if (pInst->parseBinary)
			{
				unsigned int nbytes = 0;

				/* Retrieve the instrument ID */
				pDest[nbytes++] = (pInst->instrumentID >> 8) & 0xff; //todo: define the 0x80 "measurement is ok" value.
				pDest[nbytes++] =  pInst->instrumentID       & 0xff;

				pDest += nbytes;
				*pSize += nbytes;

				/* instrumentStatus is OK */
				if (pInst->instrumentStatus == 0)
				{
					/* Measurement is OK */
					unsigned int nbytes = 0;

					/* Add the Data status of the instrument */
					pDest[nbytes++] = MEASUREMENT_DATA_OK;
					pDest += nbytes;
					*pSize    += nbytes;

					/* Add the data of the instrument */
					nbytes = pInst->parseBinary(pInst->data, pDest);
					pDest += nbytes;
					*pSize    += nbytes;
				}
				/* InsturmentStatus is NOK */
				else
				{
					/* Measurement is incorrect */
					unsigned int nbytes = 0;

					/* Add the data status of the instrument */
					pDest[nbytes++] = MEASUREMENT_DATA_NOK; //todo: define the 0xfe "measurement is NOK value.
					pDest += nbytes;
					*pSize    += nbytes;

					/* Add the data of the instrument */
					nbytes = pInst->parseBinary(pInst->data, pDest);
					pDest += nbytes;
					*pSize    += nbytes;
				}
			}
			else
			{
				/* No Parsing function or parsing function is incorrect
				 * for that instrument no need to store it in the SD card */
			}
		}

		pInst++;
	}

	return 1;
}

int getNumberOfInstrumentSpecificFromConfig(decoded_config_t * conf, int type)
{
  if (!conf)
  {
#if CONFIG_LINK_PRINT_INFOS
    xQueueSend(pPrintQueue, "[config_op] [getNumberOfInstrumentSpecificFromConfig] No configuration file.\n", 0);
#endif
	return -1;
  }
  unsigned int j = 0;
  for (unsigned int i = 0; i < conf->numberOfInstruments; i++)
  {
	if (conf->instruments[i].comMethod == type && !conf->instruments[i].pollRank)
	{
	  j++;
	}
  }
  return j;
}

int getInstrumentFromConfig(decoded_config_t * conf, instrument_config_t ** pResInst, int type)
{
  if (!conf)
  {
	return -1;
  }
  unsigned int j = 0;
  instrument_config_t * pInst = NULL;
  pInst = conf->instruments;
  for (unsigned int i = 0; i < conf->numberOfInstruments; i++)
  {
	if (pInst)
	{
	  if (pInst->comMethod == type && !pInst->pollRank)
	  {
		if (!j)
		{
		  *pResInst = pInst;
		  pInst->pollRank = 1; // to make sure that the same instrument will not be assigned
#if CONFIG_LINK_PRINT_INFOS
		  sprintf(string, "%u [config_op] [getInstrumentFromConfig] Instrument %d assigned for communication method: 0x%0X at address %p.\n",
		    (unsigned int) HAL_GetTick(), i, (unsigned int) conf->instruments[i].comMethod, (void*) &conf->instruments[i]);
		  xQueueSend(pPrintQueue, string, 0);
#endif
		}
		j++;
	  }
	}
	else
	{
	  //pInst = null
	}
	pInst++;
  }
  return j;
}

//todo : need to implement this function to be error proof, this function is currently not used
int setInstrumentToConfig(decoded_config_t * conf, unsigned int comMethod, void * data, size_t size)
{
	if(!conf)
	{
		return -1;
	}

	unsigned int numberOfInstrumentSet = 0;
	instrument_config_t * pInst = NULL;
	pInst = conf->instruments;

	for(unsigned int i = 0; i < conf->numberOfInstruments; i++, pInst++)
	{
		if(pInst)
		{
			if(pInst->comMethod == comMethod)
			{
				memcpy(conf->instruments->data, data, size);
				numberOfInstrumentSet++;
			}
		}
		else
		{
			return numberOfInstrumentSet; //error in the decoded configuration.
		}
	}

	return numberOfInstrumentSet;
}

int config_copy(decoded_config_t * pDest, decoded_config_t * pSource)
{
	memcpy(pDest, pSource, sizeof(decoded_config_t));
	return 1;
}

unsigned int config_getStoragePacketSize(decoded_config_t * pConf)
{
	assert(pConf != NULL);

	/* Pointer use to point the instrument field of the configuration */
	instrument_config_t * pInst;

	/* Temporary buffer used to get the size coming from the different instrument */
	unsigned char tmpBuffer[256];
	unsigned int size = 0;

	/* Point to the instruments field of the decoded configuration */
	pInst = pConf->instruments;

	size  += CYCLE_COUNTER_SIZE; // Sum the Cycle counter size
	size  += MEASUREMENT_DATASET_STATUS_SIZE; // Sum the status of the dataset size

	for (unsigned int i = 0; i < pConf->numberOfInstruments; i++)
	{
		if (pInst != NULL)
		{
			/* The pointer is correct check for the parsing funtion */
			if(pInst->parseBinary)
			{
				/* Parsing function is ok
				 * get the size of the instrument ID and
				 * the DATA status */
				size += INSTRUMENT_ID_SIZE; // SUM the bytes for the the instrument ID
				size += MEASUREMENT_DATA_STATUS_SIZE;

				size += pInst->parseBinary(pInst->data, tmpBuffer);
			}
			else
			{
				/* No parsing function so no need to
				 * to store the data of the instrument
				 * no need to the size of that instrument
				 * in the storage packet */
			}
		}
		pInst++;
	}
	return size;
}

void refreshBLEmoduleData(const imu_100Hz_data_t *sensorEvent, imu_100Hz_data_t * imu)
{
	float t;
	unsigned int t_ms;
	t     = 1 / 1000000.0;  // time in seconds.
	t_ms = (unsigned int) (1000.0 / 1000.0); //time in milliseconds in an unsigned int
	imu->timestamp 	            = t_ms;
	imu->rotVectors1.real 		= sensorEvent->rotVectors1.real;
	imu->rotVectors1.i 			= sensorEvent->rotVectors1.i;
	imu->rotVectors1.j 			= sensorEvent->rotVectors1.j;
	imu->rotVectors1.k 			= sensorEvent->rotVectors1.k;
	imu->gyroscope1.x			= sensorEvent->gyroscope1.x;
	imu->gyroscope1.y			= sensorEvent->gyroscope1.y;
	imu->gyroscope1.z			= sensorEvent->gyroscope1.z;
    imu->accelerometer1.x		= sensorEvent->accelerometer1.x;
    imu->accelerometer1.y		= sensorEvent->accelerometer1.y;
    imu->accelerometer1.z		= sensorEvent->accelerometer1.z;
    imu->magnetometer1.x		= sensorEvent->magnetometer1.x;
    imu->magnetometer1.y		= sensorEvent->magnetometer1.y;
    imu->magnetometer1.z		= sensorEvent->magnetometer1.z;
	imu->rotVectors2.real 		= sensorEvent->rotVectors2.real;
	imu->rotVectors2.i 			= sensorEvent->rotVectors2.i;
	imu->rotVectors2.j 			= sensorEvent->rotVectors2.j;
	imu->rotVectors2.k 			= sensorEvent->rotVectors2.k;
	imu->gyroscope2.x			= sensorEvent->gyroscope2.x;
	imu->gyroscope2.y			= sensorEvent->gyroscope2.y;
	imu->gyroscope2.z			= sensorEvent->gyroscope2.z;
    imu->accelerometer2.x		= sensorEvent->accelerometer2.x;
    imu->accelerometer2.y		= sensorEvent->accelerometer2.y;
    imu->accelerometer2.z		= sensorEvent->accelerometer2.z;
    imu->magnetometer2.x		= sensorEvent->magnetometer2.x;
    imu->magnetometer2.y		= sensorEvent->magnetometer2.y;
    imu->magnetometer2.z		= sensorEvent->magnetometer2.z;
//	sprintf(string, "%u [APP_BLEmodule%u] [BLE%uTask] Received data: %04X%04X%04X - %04X%04X%04X - %04X%04X%04X - %04X%04X%04X - %04X%04X%04X - %04X%04X%04X\n",
//	  (unsigned int) HAL_GetTick(), (unsigned int) sensorEvent->module, (unsigned int) sensorEvent->module,
//	  (unsigned int) sensorEvent->gyroscope1.x,     (unsigned int) sensorEvent->gyroscope1.y,     (unsigned int) sensorEvent->gyroscope1.z,
//	  (unsigned int) sensorEvent->accelerometer1.x, (unsigned int) sensorEvent->accelerometer1.y, (unsigned int) sensorEvent->accelerometer1.z,
//	  (unsigned int) sensorEvent->magnetometer1.x,  (unsigned int) sensorEvent->magnetometer1.y,  (unsigned int) sensorEvent->magnetometer1.z,
//	  (unsigned int) sensorEvent->gyroscope2.x,     (unsigned int) sensorEvent->gyroscope2.y,     (unsigned int) sensorEvent->gyroscope2.z,
//	  (unsigned int) sensorEvent->accelerometer2.x, (unsigned int) sensorEvent->accelerometer2.y, (unsigned int) sensorEvent->accelerometer2.z,
//	  (unsigned int) sensorEvent->magnetometer2.x,  (unsigned int) sensorEvent->magnetometer2.y,  (unsigned int) sensorEvent->magnetometer2.z);
//    xQueueSend(pPrintQueue, string, 0);

    //			(unsigned int) sensorEvent.rotVectors1.real,
    //			(unsigned int) sensorEvent.rotVectors1.i,
    //			(unsigned int) sensorEvent.rotVectors1.j,
    //			(unsigned int) sensorEvent.rotVectors1.k,

}

