/**
 * @file raw.c
 * @brief Functions around the raw configuration - decoding
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 *
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 *
 */
#include "config/raw.h"
#include <stdlib.h> //to declare malloc()
#include "usart.h"  //to declare huart5
#include "string.h" //to declare memset
#include "../common.h"
#include "../../Inc/imu_com.h"

#define RAW_DBG_PRINTF 1
#define RAW_CONFIGURATION_TEST 0

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];

extern char string[];
extern QueueHandle_t pPrintQueue;


/**
 * @fn decode_result decode_config(const uint8_t *buffer, decoded_config_t *config)
 * @brief function used to decode the configuration file send by the android device
 * @note A particular caution need to be remind all parameter value are stored as FLOAT_32
 * with the Android device APP.
 */
decode_result decode_config(const uint8_t *buffer, decoded_config_t *config)
{
  int availableBTchannel = 6;
  if(buffer == NULL && config == NULL)
  {
#if RAW_DBG_PRINTF
	xQueueSend(pPrintQueue, "[RAW] [decode_config] Buffer and configuration parameter is NULL.\n", 0);
#endif
	return EDATA;
  }
  uint16_t parameterID = 0x0000;
  uint32_t count = 0;
  if (buffer[0] == SOH)
  { /* Start of Header found */
	config->setupID   = buffer[++count];
	config->setupID   = config->setupID << 8;
	config->setupID   = config->setupID | buffer[++count];
	config->version   = buffer[++count];
	config->version   = config->version << 8;
	config->version   = config->version | buffer[++count];
	config->companyID = buffer[++count];
	config->companyID = config->companyID << 8;
	config->companyID = config->companyID | buffer[++count];
#if RAW_DBG_PRINTF
	sprintf(string, "[RAW] [decode_config] ------------------------- SETUP ID: %d\r\n", config->setupID);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[RAW] [decode_config] -------------------------- VERSION: %d\r\n", config->version);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[RAW] [decode_config] ----------------------- COMPANY ID: %d\r\n", config->companyID);
	xQueueSend(pPrintQueue, string, 0);
#endif
	if(buffer[++count] == STX)
	{ /* Start of text found */
	  config->numberOfInstruments = buffer[++count];
	  config->numberOfInstruments = config->numberOfInstruments << 8;
	  config->numberOfInstruments = config->numberOfInstruments | buffer[++count];
	  config->instruments         = malloc(config->numberOfInstruments * sizeof(config->instruments[0]));
	  memset(config->instruments, 0, config->numberOfInstruments*sizeof(config->instruments[0]));
#if RAW_DBG_PRINTF
	  sprintf(string, "[RAW] [decode_config] ------------ Number of instruments: %d\r\n", config->numberOfInstruments);
	  xQueueSend(pPrintQueue, string, 0);
#endif
	  for (uint16_t i = 0; i < config->numberOfInstruments; i++)
	  {
//#if RAW_DBG_PRINTF
//		sprintf(string, "[RAW] [decode_config] count value =  %0X\r\n", count);
//		xQueueSend(pPrintQueue, string, 0);
//#endif
		config->instruments[i].instrumentID			= buffer[++count];
		config->instruments[i].instrumentID			= config->instruments[i].instrumentID << 8;
		config->instruments[i].instrumentID			= config->instruments[i].instrumentID | buffer[++count];
		config->instruments[i].numberOfParameters	= buffer[++count];
		config->instruments[i].numberOfParameters	= config->instruments[i].numberOfParameters << 8;
		config->instruments[i].numberOfParameters	= config->instruments[i].numberOfParameters | buffer[++count];
#if RAW_DBG_PRINTF
		sprintf(string, "[RAW] [decode_config] -------------------- Instrument n°: %d\r\n", i);
		xQueueSend(pPrintQueue, string, 0);
		sprintf(string, "[RAW] [decode_config] -------------------- Instrument ID: %d\r\n", config->instruments[i].instrumentID);
		xQueueSend(pPrintQueue, string, 0);
		sprintf(string, "[RAW] [decode_config] ------------- Number of parameters: %d\r\n", config->instruments[i].numberOfParameters);
		xQueueSend(pPrintQueue, string, 0);
#endif
		for(uint16_t j = 0; j < config->instruments[i].numberOfParameters; j++)
		{
		  parameterID = buffer[++count];
		  parameterID = parameterID << 8;
		  parameterID = parameterID | buffer[++count];
		  switch (parameterID)
		  {
		  case SETUP_PRM_X :
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].x |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].x = config->instruments[i].x << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ----- HMI information - Position x: %08X = %4.0f.\r\n", config->instruments[i].x,*((float*)&config->instruments[i].x));
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_Y :
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].y |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].y = config->instruments[i].y << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ----- HMI information - Position y: %08X = %4.0f.\r\n", config->instruments[i].y,*((float*)&config->instruments[i].y));
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_R :
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].r |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].r = config->instruments[i].r << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------- HMI information - Rotation: %08X\r\n", config->instruments[i].r);
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_COMM_METHOD:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].comMethod |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].comMethod = config->instruments[i].comMethod << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ---------- Communication Interface: %08X = ", config->instruments[i].comMethod);
			xQueueSend(pPrintQueue, string, 0);
            switch(config->instruments[i].comMethod)
            {
              case SETUP_PRM_COMM_METHOD_BT:
              {
                xQueueSend(pPrintQueue, "Bluetooth.\r\n", 0);
                break;
              }
//              case SETUP_PRM_COMM_METHOD_UART:
//              {
//      		    xQueueSend(pPrintQueue, "UART (Unused for now).\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_CAN:
//              {
//      		    xQueueSend(pPrintQueue, "CAN (Unused for now).\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_SPI:
//              {
//      		    xQueueSend(pPrintQueue, "SPI (Unused for now).\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_JOYSTICK_DYNAMIC_CONTROL:
//              {
//      		    xQueueSend(pPrintQueue, "Joystick Dynamic Control.\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_JOYSTICK_PENNY_GILES:
//              {
//      		    xQueueSend(pPrintQueue, "Joystick Penny & Giles.\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_JOYSTICK_LINX:
//              {
//      		    xQueueSend(pPrintQueue, "Joystick LINX.\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_IMU:
//              {
//      		    xQueueSend(pPrintQueue, "IMU.\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_GPS:
//              {
//      		    xQueueSend(pPrintQueue, "GPS.\r\n", 0);
//                break;
//              }
//              case SETUP_PRM_COMM_METHOD_CAN_DISTANCE_SENSOR:
//              {
//      		    xQueueSend(pPrintQueue, "CAN Distance Sensor.\r\n", 0);
//                break;
//              }
              case SETUP_PRM_COMM_METHOD_RTC:
              {
      		    xQueueSend(pPrintQueue, "Real Time Clock (RTC).\r\n", 0);
      		    availableBTchannel = 6;
                break;
              }
      		  default:
      		  {
      		    xQueueSend(pPrintQueue, "Unknown communication method value.\r\n", 0);
      		    availableBTchannel = 6;
      		  }
            }
#endif
            break;
		  }
		  case SETUP_PRM_COMM_METHOD_VERSION:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].comMethodVersion |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].comMethodVersion = config->instruments[i].comMethodVersion << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] -- Communication Interface Version: %08X = Version %2.2f.\r\n", config->instruments[i].comMethodVersion,*((float*)&config->instruments[i].comMethodVersion));
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_COMM_FAIL_CONSEQUENCE:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].comFail |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].comFail = config->instruments[i].comFail << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] --- Communication Fail Consequence: %08X = ", config->instruments[i].comFail);
			xQueueSend(pPrintQueue, string, 0);
            switch(config->instruments[i].comFail)
            {
              case SETUP_PRM_COMM_FAIL_CONSEQUENCE_DO_NOTHING:
              {
      		    xQueueSend(pPrintQueue, "Do nothing.\r\n", 0);
                break;
              }
              case SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_SOFTWARE_INSTRUMENTS:
              {
      		    xQueueSend(pPrintQueue, "Stop software instruments.\r\n", 0);
                break;
              }
              case SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_VISUALISATION:
              {
      		    xQueueSend(pPrintQueue, "Stop visualisation.\r\n", 0);
                break;
              }
              case SETUP_PRM_COMM_FAIL_CONSEQUENCE_STOP_MEASUREMENTS:
              {
      		    xQueueSend(pPrintQueue, "Stop measurements.\r\n", 0);
                break;
              }
              case SETUP_PRM_COMM_FAIL_CONSEQUENCE_POWER_CUT_OFF:
              {
      		    xQueueSend(pPrintQueue, "Power cut off (if allowed by Ethical Commission).\r\n", 0);
              }
      		  default:
      		  {
      		    xQueueSend(pPrintQueue, "Unknown communication fail consequence value.\r\n", 0);
       		  }
            }
#endif
			break;
		  }
		  case SETUP_PRM_DATA_INPUT_DATATYPE:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].dataTypeInput |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].dataTypeInput = config->instruments[i].dataTypeInput << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------------------ Data Type input: %08X\r\n", config->instruments[i].dataTypeInput);
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_DATA_OUTPUT_DATATYPE:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].dataTypeOutput |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].dataTypeOutput = config->instruments[i].dataTypeOutput << 8;
			  }
			}
			if (availableBTchannel < 6)
			{ // available channel is determined in SETUP_PRM_SAMPLERATE case
			  imu_array[availableBTchannel]->outputDataType = config->instruments[i].dataTypeOutput;
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ----------------- Data Type output: %08X = ", config->instruments[i].dataTypeOutput);
			xQueueSend(pPrintQueue, string, 0);
            switch(config->instruments[i].dataTypeOutput)
            {
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_NONE:
              {
      		    xQueueSend(pPrintQueue, "None.\r\n", 0);
                break;
              }
//              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMU9AXISROTVEC:
//              {
//      		    xQueueSend(pPrintQueue, "IMU_9AXIS_ROT_VECTOR.\r\n", 0);
//                break;
//              }
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
              {
      		    xQueueSend(pPrintQueue, "IMU Quaternions + Battery voltage level.\r\n", 0);
                break;
              }
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
              {
      		    xQueueSend(pPrintQueue, "IMU Quaternions only.\r\n", 0);
      		    break;
              }
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
              {
      		    xQueueSend(pPrintQueue, "IMU Quaternions + Gyroscope + Accelerometer.\r\n", 0);
      		    break;
              }
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
              {
      		    xQueueSend(pPrintQueue, "IMU Quaternions + Gyroscope + Accelerometer @ 100Hz.\r\n", 0);
      		    break;
              }
              case SETUP_PRM_DATA_OUTPUT_DATATYPE_RTC:
              {
      		    xQueueSend(pPrintQueue, "RTC.\r\n", 0);
      		    break;
              }
      		  default:
      		  {
      		    xQueueSend(pPrintQueue, "Unknown Data Type Output value.\r\n", 0);
       		  }
            }
//			  sprintf(string, "[RAW] [decode_config] count value =  %0X\n", count);
//			  xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_DATA_INPUT_BYTES:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].dataInput |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].dataInput = config->instruments[i].dataInput << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------- Input Data Length in Bytes: %08X\r\n", config->instruments[i].dataInput);
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_DATA_OUTPUT_BYTES:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].dataOutput |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].dataOutput = config->instruments[i].dataOutput << 8;
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------ Output Data Length in Bytes: %08X\r\n", config->instruments[i].dataOutput);
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_COMM_ADDR:
		  { /* Information is stored as FLOAT32, extract it */
			union {
			        float f;
					unsigned char bytes[4];
				  } u;
			memset(&u, 0, sizeof(u));
			u.bytes[3] = buffer[++count];
			u.bytes[2] = buffer[++count];
			u.bytes[1] = buffer[++count];
			u.bytes[0] = buffer[++count];
			config->instruments[i].comAddress = (unsigned int) u.f;
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------------ Communication Address: %08X\r\n", config->instruments[i].comAddress);
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
		  case SETUP_PRM_SAMPLERATE:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].sampleRate |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].sampleRate = config->instruments[i].sampleRate << 8;
			  }
			}
			// Find available Bluetooth channel
			int j = 5;
			while (j >= 0)
			{
			  if (!imu_array[j]->macAddressAvailable)
			  {
				availableBTchannel = j;
			  }
			  j--;
			}
			imu_array[availableBTchannel]->sampleFrequency = *((float*)&config->instruments[i].sampleRate);
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ---------------------- Sample Rate: %08X = %3.0fHz.\r\n", config->instruments[i].sampleRate,*((float*)&config->instruments[i].sampleRate));
			xQueueSend(pPrintQueue, string, 0);
#endif
			break;
		  }
//		  case SETUP_PRM_SOFTWARE_FUNCTION:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].softFuncToExec |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].softFuncToExec = config->instruments[i].softFuncToExec << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] ---------------- Software Function: %08X\r\n", config->instruments[i].softFuncToExec);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_1_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID1 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID1 = config->instruments[i].sensorID1 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 1 soft instrument: %08X\r\n", config->instruments[i].sensorID1);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_2_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID2 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID2 = config->instruments[i].sensorID2 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 2 soft instrument: %08X\r\n", config->instruments[i].sensorID2);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_3_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID3 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID3 = config->instruments[i].sensorID3 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 3 soft instrument: %08X\r\n", config->instruments[i].sensorID3);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_4_ID:
//		  {
//		    for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID4 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID4 = config->instruments[i].sensorID4 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 4 soft instrument: %08X\r\n", config->instruments[i].sensorID4);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_5_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID5 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID5 = config->instruments[i].sensorID5 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 5 soft instrument: %08X\r\n", config->instruments[i].sensorID5);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_6_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID6 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID6 = config->instruments[i].sensorID6 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 6 soft instrument: %08X\r\n", config->instruments[i].sensorID6);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_SENSOR_7_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID7 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID7 = config->instruments[i].sensorID7 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 7 soft instrument: %08X\r\n", config->instruments[i].sensorID7);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//		    break;
//		  }
//		  case SETUP_PRM_SENSOR_8_ID:
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].sensorID8 |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].sensorID8 = config->instruments[i].sensorID8 << 8;
//			  }
//			}
//#if RAW_DBG_PRINTF
//			sprintf(string, "[RAW] [decode_config] - SensorID input 8 soft instrument: %08X\r\n", config->instruments[i].sensorID8);
//			xQueueSend(pPrintQueue, string, 0);
//#endif
//			break;
//		  }
//		  case SETUP_PRM_VIEW_ANGLE :
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].viewAngle |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].viewAngle = config->instruments[i].viewAngle << 8;
//			  }
//			}
//			break;
//		  }
//		  case SETUP_PRM_INTER_SENSOR_DISTANCE :
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].interSensorDistance |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].interSensorDistance = config->instruments[i].interSensorDistance << 8;
//			  }
//			}
//			break;
//		  }
//		  case SETUP_PRM_POLL_RANK :
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].pollRank |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].pollRank = config->instruments[i].pollRank << 8;
//			  }
//			}
//			break;
//		  }
//		  case SETUP_PRM_JOYSTICK_ID :
//		  {
//			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
//			{
//			  config->instruments[i].joystickID |= buffer[++count];
//			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
//			  {
//				config->instruments[i].joystickID = config->instruments[i].joystickID << 8;
//			  }
//			}
//			break;
//		  }
//		  case SETUP_PRM_PROFILE_NUMBER :
//		  {
//			/* Information is stored as FLOAT32, extract it */
//			union
//			{
//			  float f;
//			  unsigned char bytes[4];
//			} u;
//			memset(&u, 0, sizeof(u));
//			u.bytes[3] = buffer[++count];
//			u.bytes[2] = buffer[++count];
//			u.bytes[1] = buffer[++count];
//			u.bytes[0] = buffer[++count];
//			config->instruments[i].profileNumber = (unsigned int) u.f;
//			break;
//		  }
//		  case SETUP_PRM_SHORT_THROW_TRAVEL :
//		  {
//			/* Information is stored as FLOAT32, extract it */
//			union
//			{
//			  float f;
//			  unsigned char bytes[4];
//			} u;
//			memset(&u, 0, sizeof(u));
//			u.bytes[3] = buffer[++count];
//			u.bytes[2] = buffer[++count];
//			u.bytes[1] = buffer[++count];
//			u.bytes[0] = buffer[++count];
//			config->instruments[i].shortThrowTravel = (unsigned int) u.f;
//			break;
//		  }
//		  case SETUP_PRM_FORWARD_MAXIMUM_SPEED :
//		  {
//			/* Information is stored as FLOAT32, extract it */
//			union
//			{
//			  float f;
//			  unsigned char bytes[4];
//			} u;
//			memset(&u, 0, sizeof(u));
//			u.bytes[3] = buffer[++count];
//			u.bytes[2] = buffer[++count];
//			u.bytes[1] = buffer[++count];
//			u.bytes[0] = buffer[++count];
//			config->instruments[i].maximumForwardSpeed = (unsigned int) u.f;
//			break;
//		  }
//		  case SETUP_PRM_PWC_MAXIMUM_SPEED :
//		  {
//			/* Information is stored as FLOAT32, extract it */
//			union {
//			  float f;
//			  unsigned char bytes[4];
//			} u;
//			memset(&u, 0, sizeof(u));
//			u.bytes[3] = buffer[++count];
//			u.bytes[2] = buffer[++count];
//			u.bytes[1] = buffer[++count];
//			u.bytes[0] = buffer[++count];
//			config->instruments[i].pwcMaximumSpeed = (unsigned int) u.f;
//			break;
//		  }
//		  case SETUP_PRM_PWC_BOUNDARY_DISTANCE_CALIBRATION :
//		  {
//			/* Information is stored as FLOAT32, extract it */
//			union
//			{
//			  float f;
//			  unsigned char bytes[4];
//			} u;
//			memset(&u, 0, sizeof(u));
//			u.bytes[3] = buffer[++count];
//			u.bytes[2] = buffer[++count];
//			u.bytes[1] = buffer[++count];
//			u.bytes[0] = buffer[++count];
//			config->instruments[i].pwcBoundaryCalibration = (unsigned int) u.f;
//			break;
//		  }
		  case SETUP_PRM_OAS_SLOPE_START :
		  {
			break;
		  }
		  case SETUP_PRM_OAS_SLOPE_PERCENTAGE :
		  {
			break;
		  }
		  case SETUP_PRM_OAS_SLOPE_END :
		  {
			break;
		  }
		  case SETUP_PRM_BT_MAC_HIGH:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].BTMACHigh |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].BTMACHigh = config->instruments[i].BTMACHigh << 8;
			  }
			  if (v > 0)
			  {
				if (availableBTchannel < 6)
				{
				  imu_array[availableBTchannel]->mac_address[6-v] = buffer[count];
				}
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] ------- Bluetooth MAC address High: %08X\r\n", config->instruments[i].BTMACHigh);
			xQueueSend(pPrintQueue, string, 0);
//			  sprintf(string, "[RAW] [decode_config] count value =  %0X\n", count);
//			  xQueueSend(pPrintQueue, string, 0);

#endif
			break;
		  }
		  case SETUP_PRM_BT_MAC_LOW:
		  {
			for(uint16_t v = 0; v < LENGTH_IN_BYTE_PARAMETER_VALUE; v++)
			{
			  config->instruments[i].BTMACLow |= buffer[++count];
			  if(v != (LENGTH_IN_BYTE_PARAMETER_VALUE - 1))
			  {
				config->instruments[i].BTMACLow = config->instruments[i].BTMACLow << 8;
			  }
			  // Find available empty Bluetooth channel
			  if (v > 0)
			  {
				if (availableBTchannel < 6)
				{
				  imu_array[availableBTchannel]->mac_address[3-v] = buffer[count];
				}
			  }
			}
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] -------- Bluetooth MAC address Low: %08X\r\n", config->instruments[i].BTMACLow);
			xQueueSend(pPrintQueue, string, 0);
#endif
			if (availableBTchannel < 6)
			{
			  imu_array[availableBTchannel]->macAddressAvailable = 1;
#if RAW_DBG_PRINTF
			  sprintf(string, "[RAW] [decode_config] Found available Bluetooth channel: %s.\r\n", imu_array[availableBTchannel]->name);
			  xQueueSend(pPrintQueue, string, 0);
//			  sprintf(string, "[RAW] [decode_config] count value =  %0X\r\n", count);
//			  xQueueSend(pPrintQueue, string, 0);
#endif
			}
			else
			{
#if RAW_DBG_PRINTF
		      sprintf(string, "[RAW] [decode_config] No more available Bluetooth channels, rework the RAW configuration file.\r\n");
		      xQueueSend(pPrintQueue, string, 0);
#endif
			}
			break;
		  }
		  default:
		  {
#if RAW_DBG_PRINTF
			sprintf(string, "[RAW] [decode_config] Unknown parameter id: %04X.\n",parameterID);
			xQueueSend(pPrintQueue, string, 0);
#endif
			return EDATA; //-1
			break;
		  }
		  } // end of switch (parameterID)
//#if RAW_DBG_PRINTF
//		  xQueueSend(pPrintQueue, "Next parameter.\n", 0);
//#endif
		} // end of for..loop on config->instruments[i].numberOfParameters
//#if RAW_DBG_PRINTF
//		  xQueueSend(pPrintQueue, "Next instrument.\n", 0);
//#endif
	  } // end of for..loop on config->numberOfInstruments
//#if RAW_DBG_PRINTF
//	  sprintf(string, "[RAW] [decode_config] count value =  %0X\r\n", count);
//	  xQueueSend(pPrintQueue, string, 0);
//#endif
	  if(buffer[++count] != ETX)
	  {
#if RAW_DBG_PRINTF
		xQueueSend(pPrintQueue, "[RAW] [decode_config] No End of text.\n", 0);
#endif
	    return NO_ETX;
	  }
	  else
	  {
#if RAW_DBG_PRINTF
		xQueueSend(pPrintQueue, "[RAW] [decode_config] End of text found.\n", 0);
#endif
	  }
	  if(buffer[++count] != EOT)
	  {
#if RAW_DBG_PRINTF
	    xQueueSend(pPrintQueue, "[RAW] [decode_config] No End of transmission.\n", 0);
#endif
	    return NO_EOT;
	  }
	  else
	  {
#if RAW_DBG_PRINTF
	    xQueueSend(pPrintQueue, "[RAW] [decode_config] End of transmission found.\n", 0);
#endif
	  }
    } /* End of if {Start of text found} */
    else
    {
#if RAW_DBG_PRINTF
	  xQueueSend(pPrintQueue, "[RAW] [decode_config] No Start of text found.\n", 0);
#endif
	  return NO_STX;
    }
  } /* End of if {Start of Header found} */
  else
  {
#if RAW_DBG_PRINTF
	xQueueSend(pPrintQueue, "[RAW] [decode_config] No Start Of Header found.\n", 0);
#endif
	return NO_SOH;
  }
#if RAW_DBG_PRINTF
  xQueueSend(pPrintQueue, "[RAW] [decode_config] Decoding success.\n", 0);
#endif
  HAL_Delay(1000);
  return DECODE_SUCCESS;
}

//#ifdef RAW_CONFIGURATION_TEST
//
//void print_config(decoded_config_t config)
//{
//	sprintf(string, "[RAW] [print_config] \r\n --- SETUP ID   ---\r\n%d\r\n", config.setupID);
//	xQueueSend(pPrintQueue, string, 0);
//	sprintf(string, "[RAW] [print_config] \r\n --- VERSION    ---\r\n%d\r\n", config.version);
//	xQueueSend(pPrintQueue, string, 0);
//	sprintf(string, "[RAW] [print_config] \r\n --- COMPANY ID ---\r\n%d\r\n", config.companyID);
//	xQueueSend(pPrintQueue, string, 0);
//	sprintf(string, "[RAW] [print_config] \r\n --- Number of instruments ---\r\n%d\r\n", config.numberOfInstruments);
//	xQueueSend(pPrintQueue, string, 0);
//	for(uint32_t i = 0; i < config.numberOfInstruments; i ++)
//	{
//		sprintf(string, "[RAW] [print_config] \r\n--- Instrument n°: %ld 	---\r\n", i);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Instrument ID :  %d 	---\r\n", config.instruments[i].instrumentID);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Number of parameters :  %d 		---\r\n", config.instruments[i].numberOfParameters);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- HMI information ---\r\n-> Position x : %lx\r\n -> Position y : %lx\r\n-> Rotation : %lx\r\n", config.instruments[i].x, config.instruments[i].y, config.instruments[i].r);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Communication Interface :  %lx		---\r\n", config.instruments[i].comMethod);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Communication Address  :   %lx		---\r\n", config.instruments[i].comAddress);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Communication Fail Consequence : %ld ---r\n", config.instruments[i].comFail);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Sample Rate :  %ld	 	---\r\n", config.instruments[i].sampleRate);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Byte input :  %ld      ---\r\n", config.instruments[i].dataTypeInput);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Byte output :  %ld      ---\r\n", config.instruments[i].dataTypeOutput);
//		xQueueSend(pPrintQueue, string, 0);
//		sprintf(string, "[RAW] [print_config] --- Software Function :  %ld       ---\r\n", config.instruments[i].softFuncToExec);
//		xQueueSend(pPrintQueue, string, 0);
//		//osDelay(50);
//	}
//}
//
//
//
//static const uint8_t buffer_to_decode[]={0x01,0x00,0x04,0x00,0x01,0x00,0x01,0x02,0x00,0x08,0x00,0x04,0x00,0x03,0x00,
//		0x04,0x00,0x00,0x00,0x02,0x00,0x0A,0x00,0x00,0x00,0x64,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x0A,0x00,0x03,
//		0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x0A,0x00,0x00,0x00,0x64,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x0E,0x00,
//		0x05,0x00,0x04,0x00,0x00,0x00,0x10,0x00,0x05,0x00,0x00,0x00,0x01,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x0A,
//		0x00,0x00,0x00,0x64,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x12,0x00,0x05,0x00,0x04,0x00,0x00,0x00,0x10,0x00,
//		0x05,0x00,0x00,0x00,0x02,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x64,0x00,0x20,0x00,0x00,
//		0x00,0x01,0x00,0x15,0x00,0x05,0x00,0x04,0x00,0x00,0x00,0x10,0x00,0x05,0x00,0x00,0x00,0x03,0x00,0x06,0x00,
//		0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,0x64,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x18,0x00,0x05,0x00,0x04,
//		0x00,0x00,0x00,0x10,0x00,0x05,0x00,0x00,0x00,0x04,0x00,0x06,0x00,0x00,0x00,0x00,0x00,0x0A,0x00,0x00,0x00,
//		0x64,0x00,0x20,0x00,0x00,0x00,0x01,0x00,0x1B,0x00,0x08,0x00,0x0A,0x00,0x00,0x00,0x64,0x00,0x10,0x00,0x00,
//		0x00,0x04,0x00,0x20,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,0x0E,0x01,
//		0x02,0x00,0x00,0x00,0x12,0x01,0x03,0x00,0x00,0x00,0x15,0x01,0x04,0x00,0x00,0x00,0x18,0x00,0x1C,0x00,0x08,
//		0x00,0x0A,0x00,0x00,0x00,0xC8,0x00,0x10,0x00,0x00,0x00,0x04,0x00,0x20,0x00,0x00,0x00,0x01,0x01,0x00,0x00,
//		0x00,0x00,0x02,0x01,0x01,0x00,0x00,0x00,0x0E,0x01,0x02,0x00,0x00,0x00,0x12,0x01,0x03,0x00,0x00,0x00,0x15,
//		0x01,0x04,0x00,0x00,0x00,0x18,0x03,0x04};
//
//
//int raw_config_test(void)
//{
//	decoded_config_t config;
//	decode_result ret;
//
//	/* Put config in known state */
//
//	memset(&config, 0, sizeof(config));
//
//	ret = decode_config(buffer_to_decode, &config);
//
//	switch(ret)
//	{
//	case EDATA :
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] Error in argument of decode_config function", 0);
//		break;
//	}
//	case NO_SOH:
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] No Start of header", 0);
//		break;
//	}
//	case NO_STX:
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] No Start of text", 0);
//		break;
//	}
//	case NO_ETX:
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] No End of text", 0);
//		break;
//	}
//	case NO_EOT:
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] No End of transmission", 0);
//		break;
//	}
//	case DECODE_SUCCESS:
//	{
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] Decode success!\r\n", 0);
//		xQueueSend(pPrintQueue, "[RAW] [raw_config_test] Get characters to display the raw configuration\r\n", 0);
//		getchar();
//
//		print_config(config);
//
//		break;
//	}
//	}
//
//	return 1;
//
//}
//#endif /* IF RAW_CONFIGURATION_TEST */
