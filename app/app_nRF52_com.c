/*
 * app_nRF52_com.c
 *
 *  Created on: 25 Nov 2021
 *      Author: sarah
 */
#include "app_nRF52_com.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "usart.h"
#include "../lib/ring_buffer/ringbuffer_char.h"
#include <string.h>
#include "../Inc/usb_com.h"
#include "data/structures.h"
#include "app_terminal_com.h"
//#include "app_imu.h"
#include "app_BLEmodule1.h"
#include "app_BLEmodule2.h"
#include "app_BLEmodule3.h"
#include "app_BLEmodule4.h"
#include "app_BLEmodule5.h"
#include "app_BLEmodule6.h"

#include "queues/streamer_service/streamer_service_queue.h"
#include "config/parameters_id.h"
#include "nRF52_driver.h"

#define PRINTF_nRF52_COMMANAGER 1
#define PRINTF_nRF52_COM 1

/* RTOS Variables */
static osThreadId nRF52RxTaskHandle;

/* Com link protocol variables */
static nRF52_msg_t nRF52RxMsg;
static ring_buffer_t nRF52RingbufRx;

extern streamerServiceQueueMsg_t streamMsgEnabled;

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];
extern int numberOfModules;
extern int numberOfModulesMACAddressAvailable;
extern int numberOfModulesConnected;
extern int numberOfModulesCalibrated;
extern int numberOfModulesSampleRateGiven;
extern int numberOfModulesOutputDataTypeGiven;
extern int numberOfModulesSynchronized;

extern char string[];
extern QueueHandle_t pPrintQueue;

imu_module *imunRF52 = NULL;
uint8_t previous_connected_modules [6];

char command;
uint8_t moduleConnectingSequence = 1;

void nRF52ComManagerThread(const void *params);

/* Declare private functions */
static int nRF52_FindSdByte(void);
static int nRF52_HeaderPartPresent(void);
static int nRF52_BuildHeader(void);
static int nRF52_BodyPartPresent(void);
static int nRF52_BuildBody(void);
static int nRF52_csCorrect(void);
//static void rsv_data_handler(uint8_t * buf, uint8_t sensor_number, uint8_t data_format);
static int nRF52_TimedOut(unsigned int * var);
static void nRF52_RstTimeout(unsigned int * var);

static void BLEmoduleDataToSensorEvent1(int32_t data[20], imu_100Hz_data_t *sensorEvent);
static void BLEmoduleDataToSensorEvent2(int32_t data[20], imu_100Hz_data_t *sensorEvent);


imu_100Hz_data_t sensorEvent;
imu_100Hz_data_t sensorModule1Event;
imu_100Hz_data_t sensorModule2Event;
imu_100Hz_data_t sensorModule3Event;
imu_100Hz_data_t sensorModule4Event;
imu_100Hz_data_t sensorModule5Event;
imu_100Hz_data_t sensorModule6Event;

BATTERY_ARRAY battery;


void nRF52_init_rx_task(void)
{
  osDelay(100);
  nRF52_init(); // Init the decoder
#if PRINTF_nRF52_COMMANAGER
  sprintf(string, "nRF52_com_init_rx_task done. \n");
  HAL_UART_Transmit(&huart7, (uint8_t *)string, strlen(string), 25);
#endif
  memset(&nRF52RxMsg, 0, sizeof(nRF52_msg_t));
  osThreadDef(nRF52ComManager, nRF52ComManagerThread, osPriorityRealtime, 0, 1024);
  nRF52RxTaskHandle = osThreadCreate(osThread(nRF52ComManager), NULL);
}

void nRF52ComManagerThread(const void *params)
{
  osDelay(100);
  uint8_t timesFound = 0;
  int flag100HzModule1 = 0;
  int flag100HzModule2 = 0;
  int flag100HzModule3 = 0;
  int flag100HzModule4 = 0;
  int flag100HzModule5 = 0;
  int flag100HzModule6 = 0;
  for(;;)
  {
	nRF52_DECODER_RESULT nRF52Res = nRF52_prot_decoder(&nRF52RxMsg);
    if (nRF52Res == nRF52_CORRECT_FRAME)
	{
      if (nRF52RxMsg.commandType == 1)
      {// received frame is measurement data
    	if (streamMsgEnabled.action == streamerService_EnableAndroidStream)
        {
//#if PRINTF_nRF52_COMMANAGER
//          sprintf(string, "%u Payload: ", (unsigned int) HAL_GetTick()); // shorter sentence to print as much as possible...
//      	  char DUString[3];
//      	  for (int i = 7; i < nRF52RxMsg.length; i++)
//      	  {
//      	    if (strlen(string) < 149)
//      	    {
//          	  sprintf(DUString, "%02X",nRF52RxMsg.DU[i]);
//          	  strcat(string, DUString);
//      	    }
//      	  }
//       	  strcat(string,"\n");
//      	  xQueueSend(pPrintQueue, string, 0);
//#endif
    	  int32_t data[20];

    	  int16_t test_data[3*3];

    	  uint64_t timestamp;
    	  int value = 0;
    	  uint16_t start_pos = 5;
//!    	  sensorEvent.module = nRF52RxMsg.DU[3];

    	  // to find out which module we need to use: the module nr of the nRF52 data package is defined per sequence of connecting to the network,
    	  // rather then to the sequence as defined in the raw file. This connecting sequence is stored as well in the imu_module table
    	  // (in variable imu_module->connectingSequence).
    	  // The sensor nr defined in nRF52RxMsg.DU[3] is the connecting sequence nr, so if we search this in the table, then we have the right
    	  // sensor nr...

    	  int sensorNr = 0;
    	  int moduleFound = 0;
		  for (int k = 0; k < numberOfModules; k++)
		  {
			if (!moduleFound)
			{
			  if (imu_array[k]->connectingSequence == (nRF52RxMsg.DU[3]+1))
			  {
				sensorNr = imu_array[k]->number;
				moduleFound++;
			  }
			}
		  }
		  if (!moduleFound)
		  {
#if PRINTF_nRF52_COMMANAGER
            sprintf(string, "[app_nRF52_com] Connecting sequence %u not found in imu_array.\n", (unsigned int) nRF52RxMsg.DU[3]+1);
            xQueueSend(pPrintQueue, string, 0);
#endif
		  }
//    	  sensorNr = nRF52RxMsg.DU[3]+1;


    	  switch (nRF52RxMsg.DU[4]) // Data Type of nRF52 received message: Quaternions = 1, Euler = 2, RAW = 3
    	  {
  	        case 0x01:
  	        { //Quaternions
  	    	  //   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D
  	          //                 +---------+ +---------+ +---------+ +---------+ +---------------------+
  	      	  //0x73 1E 01 01 01 00 58 CF 3F E0 3E 6E FC 70 47 25 01 74 1C D1 FC B8 43 00 00 E9 FF FF FF 33
  	  		  //   |  |  |  |  | +----v----+ +----v----+ +----v----+ +----v----+ +----------v----------+  |
  	  		  //   |  |  |  |  |      |           |           |           |                 |             +--> CS
  	  		  //   |  |  |  |  |      |           |           |           |                 +--> uint64_t epochNow_Ms
  	  		  //   |  |  |  |  |      |           |           |           +--> int32_t z (rotVectors.k)
  	  		  //   |  |  |  |  |      |           |           +--> int32_t y (rotVectors.j)
  	  		  //   |  |  |  |  |      |           +--> int32_t x (rotVectors.i)
  	  		  //   |  |  |  |  |      +--> int32_t w (rotVectors.real)
  	  		  //   |  |  |  |  +--> Data Type: Quaternions = 1, Euler = 2, RAW = 3
  	  		  //   |  |  |  +--> sensor nr
  	  		  //   |  |  +--> command type: DATA
  	  		  //   |  +--> total length of packet
  	  		  //   +--> start delimiter
  	    	  for(uint8_t g = 0; g < 4; g++)
  	    	  {
        	    // data[0] = rotVectors.real
        	    // data[1] = rotVectors.i
        	    // data[2] = rotVectors.j
        	    // data[3] = rotVectors.k
//  	    	    data[g] = ((nRF52RxMsg.DU[start_pos + g*4] << 24) | (nRF52RxMsg.DU[start_pos + g*4 + 1] << 16) |
//  	    	    		   (nRF52RxMsg.DU[start_pos + g*4 + 2] << 8) | nRF52RxMsg.DU[start_pos + g*4 + 3]);

  	    		int32_t data_temp = (int32_t) ((nRF52RxMsg.DU[start_pos + g*4 + 3] << 24) | (nRF52RxMsg.DU[start_pos + g*4 + 2] << 16) |
  	    	    		   (nRF52RxMsg.DU[start_pos + g*4 + 1] << 8) | nRF52RxMsg.DU[start_pos + g*4]);
  	    	    data[g] = data_temp;

  	    	    //data[g] = (float)data[g]/(float)(1<<30);
  	    	  }
  	    	  for (uint8_t g = 0x1C; g > 0x14; g--)
  	    	  {
     	        timestamp = (timestamp << 8) | nRF52RxMsg.DU[g];
  	    	  }
	    	  //timestamp = nRF52RxMsg.DU[15] | (nRF52RxMsg.DU[16] << 8) | (nRF52RxMsg.DU[17] << 16) | (nRF52RxMsg.DU[18] << 24 |);
	    	  //sprintf(string, "[app_nRF52_com] sample frequency: %u \n", (unsigned int) imu_array[0]->sampleFrequency);
	    	  //xQueueSend(pPrintQueue, string, 0);

  	    	  /*
#if PRINTF_nRF52_COMMANAGER
#define FIXED_POINT_FRACTIONAL_BITS_QUAT    30          // Number of bits used for comma part of quaternion data
  	    	  float quatw = ((float)data[0] / (float)(1 << FIXED_POINT_FRACTIONAL_BITS_QUAT));
  	    	  float quati = ((float)data[1] / (float)(1 << FIXED_POINT_FRACTIONAL_BITS_QUAT));
  	    	  float quatj = ((float)data[2] / (float)(1 << FIXED_POINT_FRACTIONAL_BITS_QUAT));
  	    	  float quatk = ((float)data[3] / (float)(1 << FIXED_POINT_FRACTIONAL_BITS_QUAT));

          	  sprintf(string, "[app_nRF52_com] debug jona: real: %f - i: %f - j: %f - k: %f\n", quatw, quati, quatj, quatk);
          	  xQueueSend(pPrintQueue, string, 0);

          	  // OK - data komt hier goed toe en decoding OK
#endif
*/
  	          break;
  	        }
  	        case 0x02:
  	        {//Euler
  	          //todo
  	          break;
  	        }
  	        case 0x03:
  	        { //RAW
  	          //   0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
  	          //                 +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---------------------+
  	          //0x73 20 01 00 03 14 00 21 00 FE 03 01 00 F9 FF F4 FF 21 00 54 01 F2 FD 18 08 00 00 E9 FF FF FF E8
  	          //   |  |  |  |  | +-v-+ +-v-+ +-v-+ +-v-+ +-v-+ +-v-+ +-v-+ +-v-+ +-v-+ +----------v----------+  |
  	          //   |  |  |  |  |   |     |     |     |     |     |     |     |     |              |             +--> CS
  	          //   |  |  |  |  |   |     |     |     |     |     |     |     |     |              +--> uint64_t epochNow_Ms
  	          //   |  |  |  |  |   |     |     |     |     |     |     |     |     +--> int16_t Magnetometer z
  	          //   |  |  |  |  |   |     |     |     |     |     |     |     +--> int16_t Magnetometer y
  	          //   |  |  |  |  |   |     |     |     |     |     |     +--> int16_t Magnetometer x
  	          //   |  |  |  |  |   |     |     |     |     |     +--> int16_t Accelerometer z
  	          //   |  |  |  |  |   |     |     |     |     +--> int16_t Accelerometer y
  	          //   |  |  |  |  |   |     |     |     +--> int16_t Accelerometer x
  	          //   |  |  |  |  |   |     |     +--> int16_t Gyroscope z
  	          //   |  |  |  |  |   |     +--> int16_t Gyroscope y
  	          //   |  |  |  |  |   +--> int16_t Gyroscope x
  	          //   |  |  |  |  +--> Data Type: Quaternions = 1, Euler = 2, RAW = 3
  	          //   |  |  |  +--> sensor nr
  	          //   |  |  +--> command type: DATA
  	          //   |  +--> total length of packet
  	          //   +--> start delimiter
  	    	  for(uint8_t g = 0; g < 9; g++)
  	    	  {
  	    	    // data[4] = gyroscope.x
  	    	    // data[5] = gyroscope.y
  	    	    // data[6] = gyroscope.z
  	    	    // data[7] = accelerometer.x
  	    	    // data[8] = accelerometer.y
  	    	    // data[9] = accelerometer.z
  	    	    // data[10] = magnetometer.x
  	    	    // data[11] = magnetometer.y
  	    	    // data[12] = magnetometer.z
//  	    	    data[4 + g] = ((nRF52RxMsg.DU[start_pos + g*2] << 8) | nRF52RxMsg.DU[start_pos + g*2 + 1]);

  	    		int16_t data_temp = (int16_t) ((nRF52RxMsg.DU[start_pos + g*2 + 1] << 8) | nRF52RxMsg.DU[start_pos + g*2]);
  	    	    data[4 + g] = (int32_t) data_temp;

				test_data[g] = ((nRF52RxMsg.DU[start_pos + g*2 +1 ] << 8) | nRF52RxMsg.DU[start_pos + g*2]);

  	    	    //data[g] = (float)data[g]/(float)(1<<30);
  	    	  }

  	    	  /*
#define RAW_Q_FORMAT_GYR_COMMA_BITS         5           // Number of bits used for comma part of raw data.
#define RAW_Q_FORMAT_ACC_COMMA_BITS         10          // Number of bits used for comma part of raw data.
	    	    float test_gyro_x = ((float)test_data[3] / (float)(1 << RAW_Q_FORMAT_GYR_COMMA_BITS));
	    	    float test_gyro_y = ((float)test_data[4] / (float)(1 << RAW_Q_FORMAT_GYR_COMMA_BITS));
	    	    float test_gyro_z = ((float)test_data[5] / (float)(1 << RAW_Q_FORMAT_GYR_COMMA_BITS));

	    	    float test_accel_x = ((float)test_data[0] / (float)(1 << RAW_Q_FORMAT_ACC_COMMA_BITS));
	    	  	float test_accel_y = ((float)test_data[1] / (float)(1 << RAW_Q_FORMAT_ACC_COMMA_BITS));
	    		float test_accel_z = ((float)test_data[2] / (float)(1 << RAW_Q_FORMAT_ACC_COMMA_BITS));
*/

#if PRINTF_nRF52_COMMANAGER
          	  //sprintf(string, "[app_nRF52_com] debug jona: accel x: %f - y: %f - z %f\n", test_accel_x, test_accel_y, test_accel_z);
          	  //xQueueSend(pPrintQueue, string, 0);
          	  /** TODO: Tot hier komt de data goed toe **/

          	  //sprintf(string, "[app_nRF52_com] debug jona: gyro x: %f - y: %f - z %f", test_gyro_x, test_gyro_y, test_gyro_z);
          	  //xQueueSend(pPrintQueue, string, 0);
#endif


  	    	  for (uint8_t g = 0x1E; g > 0x16; g--)
  	    	  {
     	        timestamp = (timestamp << 8) | nRF52RxMsg.DU[g];
  	    	  }
  	          break;
  	        }
    	  }
    	  //switch(imu_array[nRF52RxMsg.DU[3]]->sampleFrequency) // nRF52RxMsg.DU[3] is the module nr // changed 20220407
    	  switch(imu_array[sensorNr]->sampleFrequency)
    	  {
    	    case 10:  value = 5;  break;
    	    case 25:  value = 2;  break;
    	    case 50:  value = 1;  break;
    	    case 100: value = 100;  break;
    	    default:
    	    { /* if sample frequency is not a defined value, the sample frequency is set to 50Hz */
    	      value = 1;
    	    }
    	  }
    	  if (value == 100)
    	  { // this handles following cases of imu_array[nRF52RxMsg.DU[3]]->outputDataType:
    		// case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG_100HZ:
    		// case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
    		// case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_100HZ:
    		// case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF_100HZ:
        	//switch(nRF52RxMsg.DU[3]) // sensor nr // changed 20220407
          	switch(sensorNr)
        	{
        	  case 0x01:
        	  {
                if (flag100HzModule1)
                {
//          	      sensorModule1Event.rotVectors2.real = data[0];
//          	      sensorModule1Event.rotVectors2.i    = data[1];
//          	      sensorModule1Event.rotVectors2.j    = data[2];
//          	      sensorModule1Event.rotVectors2.k    = data[3];
//      	    	  sensorModule1Event.gyroscope2.x	  = data[4];
//      	    	  sensorModule1Event.gyroscope2.y	  = data[5];
//      	    	  sensorModule1Event.gyroscope2.z	  = data[6];
//      	    	  sensorModule1Event.accelerometer2.x = data[7];
//      	    	  sensorModule1Event.accelerometer2.y = data[8];
//      	    	  sensorModule1Event.accelerometer2.z = data[9];
//      	    	  sensorModule1Event.magnetometer2.x  = data[10];
//      	    	  sensorModule1Event.magnetometer2.y  = data[11];
//      	    	  sensorModule1Event.magnetometer2.z  = data[12];
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule1Event);
      	    	  sensorHandlerBLEmodule1(&sensorModule1Event);
         	      flag100HzModule1--;
                }
                else
                {
//            	  sensorModule1Event.rotVectors1.real = data[0];
//            	  sensorModule1Event.rotVectors1.i    = data[1];
//            	  sensorModule1Event.rotVectors1.j    = data[2];
//            	  sensorModule1Event.rotVectors1.k    = data[3];
//        	      sensorModule1Event.gyroscope1.x	  = data[4];
//        	      sensorModule1Event.gyroscope1.y	  = data[5];
//        	   	  sensorModule1Event.gyroscope1.z	  = data[6];
//        	   	  sensorModule1Event.accelerometer1.x = data[7];
//        	   	  sensorModule1Event.accelerometer1.y = data[8];
//        	   	  sensorModule1Event.accelerometer1.z = data[9];
//        	   	  sensorModule1Event.magnetometer1.x  = data[10];
//        	   	  sensorModule1Event.magnetometer1.y  = data[11];
//        	   	  sensorModule1Event.magnetometer1.z  = data[12];
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule1Event);
         	      flag100HzModule1++;
                }
        	    break;
        	  }
        	  case 0x02:
        	  {
                if (flag100HzModule2)
                {
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule2Event);
        	      sensorHandlerBLEmodule2(&sensorModule2Event);
           	      flag100HzModule2--;
                }
                else
                {
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule2Event);
           	      flag100HzModule2++;
                }
        	    break;
        	  }
        	  case 0x03:
        	  {
                if (flag100HzModule3)
                {
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule3Event);
        	   	  sensorHandlerBLEmodule3(&sensorModule3Event);
           	      flag100HzModule3--;
                }
                else
                {
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule3Event);
           	      flag100HzModule3++;
                }
        	   	break;
          	  }
        	  case 0x04:
        	  {
                if (flag100HzModule4)
                {
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule4Event);
        	   	  sensorHandlerBLEmodule4(&sensorModule4Event);
           	      flag100HzModule4--;
                }
                else
                {
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule4Event);
           	      flag100HzModule4++;
                }
        	    break;
        	  }
        	  case 0x05:
        	  {
                if (flag100HzModule5)
                {
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule5Event);
        	   	  sensorHandlerBLEmodule5(&sensorModule5Event);
           	      flag100HzModule5--;
                }
                else
                {
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule5Event);
           	      flag100HzModule5++;
                }
        	    break;
        	  }
        	  case 0x06:
        	  {
                if (flag100HzModule6)
                {
      	    	  BLEmoduleDataToSensorEvent2(data, &sensorModule6Event);
        	   	  sensorHandlerBLEmodule6(&sensorModule6Event);
           	      flag100HzModule6--;
                }
                else
                {
      	    	  BLEmoduleDataToSensorEvent1(data, &sensorModule6Event);
           	      flag100HzModule6++;
                }
        	    break;
        	  }
              default:
              {
#if PRINTF_nRF52_COMMANAGER
            	  sprintf(string, "%u [app_nRF52_com] sensor event error in sending data to BLE sensor queue coming from module %u: ", (unsigned int) HAL_GetTick(), (unsigned int) nRF52RxMsg.DU[3]);
            	  char DUString[6];
            	  for (int s = 0; s < 9; s++)
                  {
            	    sprintf(DUString, "%04X ", (unsigned int) data[s]);
            	    strcat(string, DUString);
            	  }
            	  strcat(string,"\n");
            	  xQueueSend(pPrintQueue, string, 0);
#endif
              }
        	}
    	  }
    	  else
    	  { // this handles following cases of imu_array[nRF52RxMsg.DU[3]]->outputDataType:
  	        // case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
  	        // case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT: // battery voltage level not yet implemented
  	        // case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
  	        // case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF:
  	        // case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG:
	    	BLEmoduleDataToSensorEvent1(data, &sensorEvent);
        	for (uint8_t f = 0; f < value; f++)
            { // repeat same data, depending on selected sampling frequency
        	  //switch (nRF52RxMsg.DU[3]) // sensor nr // changed 20220407
        	  switch (sensorNr)
        	  {
        	    case 0x01:
        	    {
        	      sensorHandlerBLEmodule1(&sensorEvent);
        	      break;
        	    }
        	    case 0x02:
        	    {
        	      sensorHandlerBLEmodule2(&sensorEvent);
        	      break;
        	    }
        	    case 0x03:
        	    {
        	   	  sensorHandlerBLEmodule3(&sensorEvent);
        	   	  break;
        	   	}
        	   	case 0x04:
        	   	{
        	      sensorHandlerBLEmodule4(&sensorEvent);
        	      break;
        	    }
        	    case 0x05:
        	   	{
        	      sensorHandlerBLEmodule5(&sensorEvent);
        	      break;
        	    }
        	    case 0x06:
        	    {
        	      sensorHandlerBLEmodule6(&sensorEvent);
        	      break;
        	    }
                default:
            	{
#if PRINTF_nRF52_COMMANAGER
            	  sprintf(string, "%u [app_nRF52_com] sensor event error in sending data to BLE sensor queue coming from module %u: ", (unsigned int) HAL_GetTick(), (unsigned int) nRF52RxMsg.DU[3]);
            	  char DUString[6];
            	  for (int s = 0; s < 9; s++)
                  {
            	    sprintf(DUString, "%04X ", (unsigned int) data[s]);
            	    strcat(string, DUString);
            	  }
            	  strcat(string,"\n");
            	  xQueueSend(pPrintQueue, string, 0);
#endif
            	}
        	  }
            } // end of repeat same data, depending on selected sampling frequency
    	  }
        } // end of if (streamMsgEnabled.action == streamerService_EnableAndroidStream)
      } // end of received frame is measurement data
      else
      { // received frame is configuration data
		switch (nRF52RxMsg.command)
		{
		  case COMM_CMD_SET_CONN_DEV_LIST:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module SET_CONN_DEV_LIST command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_REQ_CONN_DEV_LIST:
		  {
#if PRINTF_nRF52_COMMANAGER
		    sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module REQ_CONN_DEV_LIST command received.\n",(unsigned int) HAL_GetTick());
            xQueueSend(pPrintQueue, string, 0);
#endif
			//
			// if i = 0->7 is module index, j = 0->5 is MAC address byte and offset is 7, then 7+i*10+j is pointing to the address byte per module in the nRF52RxMsg.DU[].
			//
			//                                        <-------------1------------>  <-------------2------------>  <-------------3------------>  <-------------4------------>  <-------------5------------>  <-------------6------------>  <-------------7------------>  <-------------8------------>
            //                            0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54
			//                            1  2  3  4  5  6  7  8  9 10  1  2  3  4  5  6  7  8  9 20  1  2  3  4  5  6  7  8  9 30  1  2  3  4  5  6  7  8  9 40  1  2  3  4  5  6  7  8  9 50  1  2  3  4  5  6  7  8  9 60  1  2  3  4  5  6  7  8  9 70  1  2  3  4  5  6  7  8  9 80  1  2  3  4  5
			//example received buffer: 0x73 55 02 02 FF FF 02 0D 11 29 0F 2C D4 00 00 00 02 C6 D8 42 14 35 E8 00 FF FF 02 FF FF FF FF FF FF 00 FF FF 02 FF FF FF FF FF FF 00 FF FF 02 FF FF FF FF FF FF 00 FF FF 02 FF FF FF FF FF FF 00 FF FF 02 FF FF FF FF FF FF 00 FF FF 02 FF FF FF FF FF FF 00 71
			//                            |  |  |  |  ?  ?  ? +-------v-------+  ?          +-------v-------+             +-------v-------+             +-------v-------+             +-------v-------+             +-------v-------+             +-------v-------+             +-------v-------+     |
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             |                             |                             |                             |             +--> Check Sum
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             |                             |                             |                             |
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             |                             |                             |                             +--> MAC address module 8
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             |                             |                             +--> MAC address module 7
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             |                             +--> MAC address module 6
			//                            |  |  |  |  |  |  |         |                             |                             |                             |                             +--> MAC address module 5
			//                            |  |  |  |  |  |  |         |                             |                             |                             +--> MAC address module 4
			//                            |  |  |  |  |  |  |         |                             |                             +--> MAC address module 3
			//                            |  |  |  |  |  |  |         |                             +--> MAC address module 2
			//                            |  |  |  |  |  |  |         +--> MAC address module 1
			//                            |  |  |  |  |  |  +--> conn_handle?? maar dat kan niet, want conn_handle is een uint16_t
			//                            |  |  |  |  |  +--> according to ble_gap_addr_t Struct Reference, this is uint8_t ble_gap_addr_t::addr_type. Following possibilities are defined in nordic api:
			//                            |  |  |  |  |       #define BLE_GAP_ADDR_TYPE_PUBLIC                        0x00 Public address.
			//                            |  |  |  |  |       #define BLE_GAP_ADDR_TYPE_RANDOM_STATIC                 0x01 Random private non-resolvable address.
			//                            |  |  |  |  |       #define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_RESOLVABLE     0x02 Random private resolvable address.
			//                            |  |  |  |  |       #define BLE_GAP_ADDR_TYPE_RANDOM_PRIVATE_NON_RESOLVABLE 0x03 Random static address.
			//                            |  |  |  |  +--> according to ble_gap_addr_t Struct Reference, this is uint8_t ble_gap_addr_t::addr_id_peer. Only valid for peer addresses.
			//                            |  |  |  |       Reference to peer in device identities list (as set with sd_ble_gap_device_identities_set) when peer is using privacy.
			//                            |  |  |  +--> command --> command_type_byte_t:
			//                            |  |  |                    SET_CONN_DEV_LIST    = 0x1
			//                            |  |  |                   *REQ_CONN_DEV_LIST    = 0x2
			//                            |  |  |                    START_MEASUREMENT    = 0x3  All sensors will start to measure at the same time
			//                            |  |  |                    STOP_MEASUREMENT     = 0x4  All sensors will stop to measure at the same time
			//                            |  |  |                    SET_MEAS_DATA_TYPE   = 0x5  Not possible to give different sensors different settings, the settings are stored in the DCU (nRF) and send to all connected sensors when START_MEASUREMENT command is received, after x ms the measurements start synchronously.
			//                            |  |  |                    TIME_SYNC_SENSORS    = 0x6
			//                            |  |  |                    SET_SAMPLE_FREQUENCY = 0x7  This should be done per sensor, for instance when different sensors are connected like IMU and EMG. This is not implemented at the moment
			//                            |  |  |                    START_CALIBRATION    = 0x8  ?? Can this be done per sensor ?? Not all sensors need calibration... + list of calibrated sensors
			//                            |  |  |                    CMD_RESET            = 0x9  This resets the configuration profile on the nRF, can also be changed by overriding existing settings.
			//                            |  |  |                    REQ_BATTERY_LEVEL    = 0xA  Battery level of all connected sensors
			//                            |  |  +--> command Type --> command_byte_t:  DATA   = 0x1
			//                            |  |                                        *CONFIG = 0x2
			//                            |  +--> packet_len is the number of elements of the COMPLETE package, including SD, Command, Packet_length and CS. In this case 0x55 = 85 elements.
			//                            +--> start delimiter
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module REQ_CONN_DEV_LIST command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
			module_status_overview();
#endif
		  }
		  break;
		  case COMM_CMD_START:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module START command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_STOP:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module STOP command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_MEAS:
	      {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module MEAS command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_SYNC:
		  {
			//                             0  1  2  3  4  5  6
			// Example received buffer: 0x73 07 02 06 02 01 73
			// Example received buffer: 0x73 07 02 06 00 01 71
			//                             |  |  |  |  |  |  |
			//                             |  |  |  |  |  |  +--> CS
			//                             |  |  |  |  |  +--> always 1?
			//                             |  |  |  |  +--> module nr, from 0 to 7.
			//                             |  |  |  +--> COMM_CMD_SYNC
			//                             |  |  +--> command type: CONFIG
			//                             |  +--> total length of packet
			//                             +--> start delimiter
#if PRINTF_nRF52_COM
            char DUString[4];
            sprintf(string, "nRF52 rec buffer: 0x");
            xQueueSend(pPrintQueue, string, 0);
            int loper = 0;
            for (int i = 0; i < nRF52RxMsg.length; i++)
            {
              sprintf(DUString, "%02X ",nRF52RxMsg.DU[i]);
              strcat(string, DUString);
              if (loper++ > 16)
              {
              	loper = 0;
                xQueueSend(pPrintQueue, string, 0);
                sprintf(string, "");
              }
            }
            sprintf(DUString, "  \n");
            strcat(string, DUString);
            xQueueSend(pPrintQueue, string, 0);
#endif
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module SYNC command received for module %01X.\n",(unsigned int) HAL_GetTick(), nRF52RxMsg.DU[4]);
			xQueueSend(pPrintQueue, string, 0);
#endif
			imu_array[nRF52RxMsg.DU[4]]->is_synchronized = nRF52RxMsg.DU[5];
			// check if all modules are calibrated:
			timesFound = 0;
			for (int k = 0; k < numberOfModules; k++)
			{
			  if (imu_array[k]->is_synchronized == 0x01)
			  {
				timesFound++;
			  }
			  if (timesFound == numberOfModules)
			  {
				numberOfModulesSynchronized = numberOfModules;
#if PRINTF_nRF52_COMMANAGER
				sprintf(string, "[app_nRF52_com] [nRF52ComManagerThread] All BLE modules synchronized.\n");
				xQueueSend(pPrintQueue, string, 0);
#endif
			  }
			}
		  }
		  break;
		  case COMM_CMD_FREQUENCY:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module FREQUENCY command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_CALIBRATE:
		  {
			//                             0  1  2  3  4  5  6
			// Example received buffer: 0x73 07 02 08 00 01 7F
			//                             |  |  |  |  |  |  |
			//                             |  |  |  |  |  |  +--> CS
			//                             |  |  |  |  |  +--> calibration status: normal calibration sequence is 1 -> 3 -> 4 -> 2
			//                             |  |  |  |  |                           1 = COMM_CMD_CALIBRATION_START
			//                             |  |  |  |  |                           2 = COMM_CMD_CALIBRATION_MAG_DONE (so full calibration done)
			//                             |  |  |  |  |                           3 = COMM_CMD_CALIBRATION_GYRO_DONE
			//                             |  |  |  |  |                           4 = COMM_CMD_CALIBRATION_ACCEL_DONE
			//                             |  |  |  |  +--> module nr, from 0 to 7.
			//                             |  |  |  +--> COMM_CMD_CALIBRATE
			//                             |  |  +--> command type: CONFIG
			//                             |  +--> total length of packet
			//                             +--> start delimiter
#if PRINTF_nRF52_COMMANAGER
			switch(nRF52RxMsg.DU[5])
			{
			  case COMM_CMD_CALIBRATION_START:
			  {
			    sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Calibration started for %s.\n",(unsigned int) HAL_GetTick(),imu_array[nRF52RxMsg.DU[4]]->name);
				xQueueSend(pPrintQueue, string, 0);
				break;
			  }
			  case COMM_CMD_CALIBRATION_DONE:
			  {
			    sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Magnetometer calibration done for %s.\n",(unsigned int) HAL_GetTick(),imu_array[nRF52RxMsg.DU[4]]->name);
			    xQueueSend(pPrintQueue, string, 0);
				break;
			  }
			  case COMM_CMD_CALIBRATION_GYRO_DONE:
			  {
				sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Gyroscope calibration done for %s.\n",(unsigned int) HAL_GetTick(),imu_array[nRF52RxMsg.DU[4]]->name);
				xQueueSend(pPrintQueue, string, 0);
				break;
			  }
			  case COMM_CMD_CALIBRATION_ACCEL_DONE:
			  {
				sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Accelerometer calibration done for %s.\n",(unsigned int) HAL_GetTick(),imu_array[nRF52RxMsg.DU[4]]->name);
				xQueueSend(pPrintQueue, string, 0);
				break;
			  }
			  default:
			  {
			    sprintf(string, "[app_init] [initThread] Try to calibrate gyroscope of connected modules. None defined value in imu_array[i]->is_calibrated.\n");
				xQueueSend(pPrintQueue, string, 0);
			  }
			}
#endif
			imu_array[nRF52RxMsg.DU[4]]->is_calibrated = nRF52RxMsg.DU[5];
			// check if all modules are calibrated:
			timesFound = 0;
			for (int k = 0; k < numberOfModules; k++)
			{
			  if (imu_array[k]->is_calibrated == 0x02)
			  {
				timesFound++;
			  }
			  if (timesFound == numberOfModules)
			  {
				numberOfModulesCalibrated = numberOfModules;
#if PRINTF_nRF52_COMMANAGER
				sprintf(string, "[app_nRF52_com] [nRF52ComManagerThread] All BLE nodes calibrated.\n");
				xQueueSend(pPrintQueue, string, 0);
#endif
			  }
			}
	      }
		  break;
		  case COMM_CMD_RESET:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module RESET command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
          case COMM_CMD_REQ_BATTERY_LEVEL:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module REQ_BATTERY_LEVEL command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif

			memcpy(&battery, &nRF52RxMsg.DU[4], sizeof(battery));

#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] battery: %.2f - %.2f - %.2f - %.2f - %.2f - %.2f",
					(unsigned int) HAL_GetTick(),
					battery.batt[0].voltage,
					battery.batt[1].voltage,
					battery.batt[2].voltage,
					battery.batt[3].voltage,
					battery.batt[4].voltage,
					battery.batt[5].voltage);
			xQueueSend(pPrintQueue, string, 0);
#endif

		  }
		  break;
		  case COMM_CMD_OK:
          {
			// Example received buffer: 0x73 07 02 0B FF 01 83
			//                             |  |  |  |  |  |  |
			//                             |  |  |  |  |  |  +--> CS
			//                             |  |  |  |  |  +--> refers to the requested command where this OK is referring to
			//                             |  |  |  |  +--> not in use for the moment, but it would be good to give here the command reference where the OK is pointed to.
			//                             |  |  |  +--> COMM_CMD_OK
			//                             |  |  +--> command type: CONFIG
			//                             |  +--> total length of packet
			//                             +--> start delimiter
#if PRINTF_nRF52_COMMANAGER
			switch(nRF52RxMsg.DU[5])
			{
			   case COMM_CMD_SET_CONN_DEV_LIST:
			   {
				  numberOfModulesMACAddressAvailable = numberOfModules;
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] MAC addresses confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_REQ_CONN_DEV_LIST:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Device list request confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_START:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Start measurement command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_STOP:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Stop measurement command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_MEAS:
			   {
				  // example received buffer: 0x73 07 02 0B FF 05 87
				  //                             |  |  |  |  |  |  |
				  //                             |  |  |  |  |  |  +--> CS
				  //                             |  |  |  |  |  +--> refers to the requested command MEAS
				  //                             |  |  |  |  +--> not in use for the moment, but it would be good to give here the command reference where the OK is pointed to.
				  //                             |  |  |  +--> COMM_CMD_OK
				  //                             |  |  +--> command type: CONFIG
				  //                             |  +--> total length of packet
				  //                             +--> start delimiter
				  numberOfModulesOutputDataTypeGiven = numberOfModules;
				  for (int k = 0; k < numberOfModules; k++)
				  {
					 imu_array[k]->outputDataTypeGiven = 0x01;
				  }
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Output data type command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_SYNC:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Synchronization command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_FREQUENCY:
			   {
				  // example received buffer: 0x73 07 02 0B FF 07 85
				  //                             |  |  |  |  |  |  |
				  //                             |  |  |  |  |  |  +--> CS
				  //                             |  |  |  |  |  +--> refers to the requested command set Frequency
				  //                             |  |  |  |  +--> not in use for the moment, but it would be good to give here the command reference where the OK is pointed to.
				  //                             |  |  |  +--> COMM_CMD_OK
				  //                             |  |  +--> command type: CONFIG
				  //                             |  +--> total length of packet
				  //                             +--> start delimiter
				  numberOfModulesSampleRateGiven = numberOfModules;
				  for (int k = 0; k < numberOfModules; k++)
				  {
					 imu_array[k]->sampleRateGiven = 0x01;
				  }
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Sample rate command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_CALIBRATE:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Calibration command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_RESET:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Reset command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_REQ_BATTERY_LEVEL:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Battery level command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_OK:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] OK command received by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_UNKNOWN:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Unknown command received by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   case COMM_CMD_SENSOR_CONNECT_STATUS:
			   {
				  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Sensor connect status command confirmed by nRF52 module.\n",(unsigned int) HAL_GetTick());
				  xQueueSend(pPrintQueue, string, 0);
				  break;
			   }
			   default:
			   {
				  sprintf(string, "[app_init] [initThread] None defined value in nRF52 OK command feedback.\n");
				  xQueueSend(pPrintQueue, string, 0);
			   }
            }
#endif
          }
		  break;
          case COMM_CMD_UNKNOWN:
		  {
#if PRINTF_nRF52_COMMANAGER
			sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Unknown command received.\n",(unsigned int) HAL_GetTick());
			xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		  break;
		  case COMM_CMD_SENSOR_CONNECT_STATUS:
          {
		     //                             0  1  2  3  4  5  6  7  8  9 10  1  2
			 //                          0x73 0D 02 0D 01 02 0D 11 29 0F 2C D4 B0
			 //                          0x73 0D 02 0D 01 02 C6 D8 42 14 35 E8 E7
			 // Example received buffer: 0x73 0D 02 0D 01 02 C6 D8 42 14 35 E8 E7
			 //                             |  |  |  |  |  |           |        |
			 //                             |  |  |  |  |  |           |        +--> CS
			 //                             |  |  |  |  |  |           +--> MAC ADDRESS
			 //                             |  |  |  |  |  +--> Connection status? 1 = not connected; 2 = connected?
			 //                             |  |  |  |  +--> Sensor nr?
			 //                             |  |  |  +--> COMM_CMD_SENSOR_CONNECT_STATUS
			 //                             |  |  +--> command type: CONFIG
			 //                             |  +--> total length of packet
			 //                             +--> start delimiter

			 // determine which module is connected
			 //
			 uint8_t lookupMacAddress [6];
			 uint8_t timesFF;
			 uint8_t timesZZ;
			 uint8_t validMacAddress = 1;
			 uint8_t timesFound = 0;
			 // step 1: take the MAC address of the module in the received stream and check if this is different than 00.00.00.00.00.00 and FF.FF.FF.FF.FF.FF
			 timesFF = 0;
			 timesZZ = 0;
			 for (int j = 0; j < 6; j++)
			 {
			    lookupMacAddress[j] = nRF52RxMsg.DU[6+j];
			    if (lookupMacAddress[j] == 0xFF)
				{
				   timesFF++;
				}
				if (lookupMacAddress[j] == 0x00)
				{
				   timesZZ++;
				}
			 }
			 if (timesFF == 0x06)
			 {
				validMacAddress = 0;
			 }
			 if (timesZZ == 0x06)
			 {
			    validMacAddress = 0;
			 }
			 // step 2: check if this lookupMacAddress is existing in the imu_array:
			 if (validMacAddress)
			 {
//        	    sprintf(string, "timesFF = %d, timesZZ = %d, valid lookup MAC Address:", timesFF, timesZZ);
//              for (int j = 0; j < 6; j++)
//              {
//                 if (strlen(string) < 147)
//                 {
//          	   	  sprintf(DUString, "%02X",lookupMacAddress[j]);
//          	   	  strcat(string, DUString);
//                 }
//              }
//              xQueueSend(pPrintQueue, string, 0);
			    for (int k = 0; k < numberOfModules; k++)
				{
				  timesFound = 0;
				  for (int j = 0; j < 6; j++)
				  {
					if (imu_array[k]->mac_address[j] == lookupMacAddress[5-j])
					{
					  timesFound++;
					}
				  }
				  if (timesFound == 0x06)
				  {
					if (!imu_array[k]->connected)
					{
					  imu_array[k]->connected = 1;
					  imu_array[k]->connectingSequence = moduleConnectingSequence;
					  moduleConnectingSequence++;
#if PRINTF_nRF52_COMMANAGER
					  sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] Sensor connect status received for module %02X, with connecting sequence %02X.\n",(unsigned int) HAL_GetTick(),imu_array[k]->number, imu_array[k]->connectingSequence);
					  xQueueSend(pPrintQueue, string, 0);

#endif
					}
			      }
				}
             }
			 else
			 {
				  xQueueSend(pPrintQueue, "No valid lookup MAC Address.\n", 0);
			 }
			 // step 3: check if all modules are connected, if so, start calibration.
			 timesFound = 0;
			 for (int k = 0; k < numberOfModules; k++)
			 {
			    if (imu_array[k]->connected)
				{
				   timesFound++;
				}
			 }
			 if (timesFound == numberOfModules)
			 {
				numberOfModulesConnected = numberOfModules;
				// start calibration:
#if PRINTF_nRF52_COMMANAGER
				sprintf(string, "[app_nRF52_com] [nRF52ComManagerThread] All BLE nodes connected, start calibration process of each module.\n");
				xQueueSend(pPrintQueue, string, 0);
#endif
				osDelay(2000);
				// comm_calibrate();
				for (int i = 0; i < numberOfModules; i++)
				 {
				   imu_array[i]->is_calibrated = COMM_CMD_CALIBRATION_DONE;
				 }
				numberOfModulesCalibrated = numberOfModules;
				//HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
				//HAL_GPIO_WritePin(LED_BUSY_GPIO_Port, LED_BUSY_Pin, GPIO_PIN_SET);
			 }
          }
		  break;
		  default:
		  {
#if PRINTF_nRF52_COMMANAGER
			 sprintf(string, "%u [app_nRF52_com] [nRF52ComManagerThread] nRF52 module unknown command received: 0x%02X.\n",(unsigned int) HAL_GetTick(),nRF52RxMsg.command);
			 xQueueSend(pPrintQueue, string, 0);
#endif
		  }
		}
      }
	}
    else
    {
      if (nRF52Res == nRF52_INCORRECT_FRAME)
      {
#if PRINTF_nRF52_COM
        char DUString[4];
        sprintf(string, "nRF52 rec buffer: 0x");
        xQueueSend(pPrintQueue, string, 0);
        int loper = 0;
        for (int i = 0; i < nRF52RxMsg.length; i++)
        {
          sprintf(DUString, "%02X ",nRF52RxMsg.DU[i]);
          strcat(string, DUString);
          if (loper++ > 16)
          {
          	loper = 0;
            xQueueSend(pPrintQueue, string, 0);
            sprintf(string, "");
          }
        }
        sprintf(DUString, "  \n");
        strcat(string, DUString);
        xQueueSend(pPrintQueue, string, 0);
#endif
      }
    }
	osDelay(2); // every 2ms
  }
}

void nRF52_RxHandler(char c)
{
    // Queue in ring buffer the rx'd byte
    ring_buffer_queue(&nRF52RingbufRx, c);
}

void nRF52_init()
{
    ring_buffer_init(&nRF52RingbufRx);
    nRF52_rst_msg(&nRF52RxMsg);
}

void nRF52_rst_msg(nRF52_msg_t * nRF52Msg)
{
  if (nRF52Msg != NULL)
  {
	nRF52Msg->command = 0;
	nRF52Msg->length = 0;
	nRF52Msg->CS = 0;
    memset(nRF52Msg->DU, 0, MAX_nRF52_PAYLOAD_LENGTH);
  }
}

nRF52_DECODER_RESULT nRF52_prot_decoder(nRF52_msg_t * nRF52Msg)
{
  static nRF52_PARSER_STATE nRF52State = nRF52_IDLE;
  nRF52_DECODER_RESULT nRF52Res = nRF52_NO_MSG;
  static unsigned int timeout = 0;

  switch (nRF52State)
  {
    case nRF52_IDLE: // check if start byte is found
    {
      if (nRF52_FindSdByte())
      {
//#if PRINTF_nRF52_COM
//        sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_IDLE] found nRF52 Start Delimiter.\n",(unsigned int) HAL_GetTick());
//	    xQueueSend(pPrintQueue, string, 0);
//#endif
//        nRF52_RstTimeout(&timeout);
//        nRF52_rst_msg(&tmpnRF52Msg);
	    nRF52State = nRF52_BUILDING_HEADER;
      }
      else
      {
    	nRF52Res = nRF52_NO_MSG;
        break;
      }
    }

    /* no break */

    case  nRF52_BUILDING_HEADER: // check if command and length is available
    {

//#if PRINTF_nRF52_COM
//      sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Start building header.\n",(unsigned int) HAL_GetTick());
//      xQueueSend(pPrintQueue, string, 0);
//#endif
      if (nRF52_HeaderPartPresent())
      {
//#if PRINTF_nRF52_COM
//        sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] nRF52 header part present.\n",(unsigned int) HAL_GetTick());
//	    xQueueSend(pPrintQueue, string, 0);
//#endif
//        nRF52_RstTimeout(&timeout);
        if (nRF52_BuildHeader())
        {
          /* Could build a header */
//#if PRINTF_nRF52_COM
//          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Command received is 0x%02X\n",(unsigned int) HAL_GetTick(), nRF52RxMsg.command);
//	      xQueueSend(pPrintQueue, string, 0);
//          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Length of body is %d\n",(unsigned int) HAL_GetTick(), nRF52RxMsg.length);
//	      xQueueSend(pPrintQueue, string, 0);
//#endif
	      nRF52State = nRF52_BUILDING_BODY;
	      nRF52Res = nRF52_IN_PROGRESS;
        }
        else
        {
          /* Invalid header */

          if (nRF52_TimedOut(&timeout))
          {
#if PRINTF_nRF52_COM
            sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Time out in building header.\n",(unsigned int) HAL_GetTick());
    	    xQueueSend(pPrintQueue, string, 0);
            sprintf(string,"Less than 2 elements in the buffer, back to idle\n");
    	    xQueueSend(pPrintQueue, string, 0);
#endif
  			//ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
          }
	      nRF52State = nRF52_IDLE;
	      nRF52Res = nRF52_NO_MSG;
          break;
        }
      }
      else
      {
        if (nRF52_TimedOut(&timeout))
        {

#if PRINTF_nRF52_COM
          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Time out in building header.\n",(unsigned int) HAL_GetTick());
    	  xQueueSend(pPrintQueue, string, 0);
          sprintf(string,"Less than 2 elements in the buffer, back to idle\n");
    	  xQueueSend(pPrintQueue, string, 0);
#endif
    	  //ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
        }
        else
        {
//#if PRINTF_nRF52_COM
//          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_HEADER] Wrong header.\n",(unsigned int) HAL_GetTick());
//    	  xQueueSend(pPrintQueue, string, 0);
//#endif
        }
        nRF52State = nRF52_IDLE;
	    nRF52Res = nRF52_NO_MSG;
        break;
      }
    }

    /* no break */

    case  nRF52_BUILDING_BODY: // check if payload is available (length bytes)
    {

//#if PRINTF_nRF52_COM
//      sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_BODY] start building body.\n",(unsigned int) HAL_GetTick());
//      xQueueSend(pPrintQueue, string, 0);
//#endif
      if (nRF52_BodyPartPresent())
      {
//#if PRINTF_nRF52_COM
//        sprintf(string,"nRF52_BodyPartPresent, %u elements in ringbuffer, >= %d length.\n",(unsigned int) ring_buffer_num_items(&nRF52RingbufRx),(nRF52RxMsg.length));
//	    xQueueSend(pPrintQueue, string, 0);
//#endif
        if (nRF52_BuildBody())
        {
//#if PRINTF_nRF52_COM
//          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_BODY] Going to check frame\n",(unsigned int) HAL_GetTick());
//	      xQueueSend(pPrintQueue, string, 0);
//#endif
	      nRF52State = nRF52_CHECKING_FRAME;
	      nRF52Res = nRF52_IN_PROGRESS;
        }
        else
        {
	      nRF52State = nRF52_IDLE;
	      nRF52Res = nRF52_NO_MSG;

          if (nRF52_TimedOut(&timeout))
          {
#if PRINTF_nRF52_COM
            sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_BODY] Time out in building body. smState = CPL_IDLE.\n",(unsigned int) HAL_GetTick());
	        xQueueSend(pPrintQueue, string, 0);
	        sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_BODY] Back to Idle\n",(unsigned int) HAL_GetTick());
		    xQueueSend(pPrintQueue, string, 0);
#endif
			//ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
          }
          break;
        }
        nRF52_RstTimeout(&timeout);
      }
      else
      {
		//ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
        nRF52State = nRF52_IDLE;
        nRF52Res = nRF52_NO_MSG;

        if (nRF52_TimedOut(&timeout))
        {
#if PRINTF_nRF52_COM
          sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_BUILDING_BODY] Time out in building body. smState = CPL_IDLE.\n",(unsigned int) HAL_GetTick());
	      xQueueSend(pPrintQueue, string, 0);
          sprintf(string,"nRF52 body part not present: expected %d elements, but only %u elements available, back to idle.\n",(nRF52RxMsg.length),(unsigned int) ring_buffer_num_items(&nRF52RingbufRx));
	      xQueueSend(pPrintQueue, string, 0);
#endif
	      //ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
        }

        break;
      }
    }

    /* no break */

    case  nRF52_CHECKING_FRAME: // calculate checksum and compare
    {
//#if PRINTF_nRF52_COM
//      char DUString[4];
//      sprintf(string, "nRF52 rec buffer: 0x");
//      xQueueSend(pPrintQueue, string, 0);
//      int loper = 0;
//      for (int i = 0; i < nRF52RxMsg.length; i++)
//      {
//        sprintf(DUString, "%02X ",nRF52RxMsg.DU[i]);
//        strcat(string, DUString);
//        if (loper++ > 16)
//        {
//        	loper = 0;
//            xQueueSend(pPrintQueue, string, 0);
//            sprintf(string, "");
//        }
//      }
//      sprintf(DUString, "  \n");
//      strcat(string, DUString);
//      xQueueSend(pPrintQueue, string, 0);
//#endif
      if (nRF52_csCorrect())
      {
//#if PRINTF_nRF52_COM
//        sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_CHECKING_FRAME] Check Sum correct.\n",(unsigned int) HAL_GetTick());
//	    xQueueSend(pPrintQueue, string, 0);
//#endif
	    nRF52Res = nRF52_CORRECT_FRAME;
	    nRF52State = nRF52_IDLE;
        break;
      }
      else
      {
#if PRINTF_nRF52_COM
        sprintf(string,"%u [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_CHECKING_FRAME] Check Sum not correct, back to Idle.\n",(unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
//		ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, ring_buffer_num_items(&nRF52RingbufRx));
	    nRF52State = nRF52_IDLE;
	    nRF52Res = nRF52_INCORRECT_FRAME;
      }
    }
  }
  return nRF52Res;
}

static int nRF52_FindSdByte(void)
{
  char rxdByte;
  rxdByte = 0;
  while (!ring_buffer_is_empty(&nRF52RingbufRx))
  {
	ring_buffer_peek(&nRF52RingbufRx, &rxdByte, 0);
    if (rxdByte == NRF52_START_DELIMITER)
    {
//#if PRINTF_nRF52_COM
//      sprintf(string,"%u [app_nRF52_com] [nRF52_FindSdByte] Found start byte.\n",(unsigned int) HAL_GetTick());
//	  xQueueSend(pPrintQueue, string, 0);
//#endif
      // Sometimes, a second start delimiter occurs, see example:
	  //                                     0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F
      //                                                   +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---+ +---------------------+
      // 20220219 17:48:43.507 -> 139193 [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_CHECKING_FRAME] Check Sum not correct, back to Idle.
      // 17:48:43.547 -> nRF52 rec buffer:0x73 73 20 01 00 03 CE FF 1E 00 11 04 01 00 F3 FF EC FF D9 FF 22 01 BC FD F2 0F 01 00 E9 FF FF FF DB
      //                                        +---> 2x 73!
      //                                    73 20 01 00 03 BD FF 1D 00 00 04 FC FF 00 00 FE FF EF FF 24 01 BE FD 06 10 01 00 E9 FF FF FF 7F
      //                                    73 20 01 00 03 C7 FF 26 00 FE 03 09 00 F9 FF F2 FF E5 FF 22 01 B0 FD 1A 10 01 00 E9 FF FF FF D9
      //                                    73 20 01 00 03 C3 FF 1A 00 0B 04 0B 00 FB FF F6 FF E3

      // 20220219 17:51:37.513 -> 313143 [app_nRF52_com] [nRF52_prot_decoder] [nRF52State = nRF52_CHECKING_FRAME] Check Sum not correct, back to Idle.
      //                                  0x73 73 20 01 00 03 7A FF 88 FF A5 04 AE 0E 27 FD 08 06 C1 FF 61 FF 7A FD A2 B7 03 00 E9 FF FF FF 51
      //                                        +---> 2x 73!
      //                                    73 20 01 00 03 81 FF 86 FF 6D 04 E5 0E 48 00 AA 06 B7 FF 7B FF 6C FD B6 B7 03 00 E9 FF FF FF 79
      //                                    73 20 01 00 03 71 FF 70 FF 4F 04 3B 10 38 02 A7 06 9D FF 89 FF 86 FD CA B7 03 00 E9 FF FF FF AC
      //                                    73 20 01 00 03 2F FF AC FF 41 04 EC 12 5D 07 30 05 72
      // check if the next byte in the ringbuffer is also the start delimiter. If so, remove.
      ring_buffer_peek(&nRF52RingbufRx, &rxdByte, 1);
      if (rxdByte == NRF52_START_DELIMITER)
      {
    	ring_buffer_dequeue(&nRF52RingbufRx, &rxdByte);
      }
      return 1;
    }
    else
    {
      ring_buffer_dequeue(&nRF52RingbufRx, &rxdByte);
    }
  }
  return 0;
}
static int nRF52_HeaderPartPresent(void)
{
  char rxdByte;
  rxdByte = 0;
  ring_buffer_peek(&nRF52RingbufRx, &rxdByte, 2);
  if (ring_buffer_num_items(&nRF52RingbufRx) >= 2) // packet_len byte + command byte
  {
	if (( rxdByte == 1) || (rxdByte == 2)) // 1 is DATA, 2 is CONFIG
	{
	  return 1;
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

static int nRF52_BuildHeader(void)
{
  char rxdByte[3];
  memset(rxdByte, 0, 3);
  if (ring_buffer_num_items(&nRF52RingbufRx) >= 3)
  {
    ring_buffer_peek(&nRF52RingbufRx, &rxdByte[0], 1);
    ring_buffer_peek(&nRF52RingbufRx, &rxdByte[1], 2);
    ring_buffer_peek(&nRF52RingbufRx, &rxdByte[2], 3);
    nRF52RxMsg.length = (int) rxdByte[0];
    nRF52RxMsg.commandType = rxdByte[1];
    nRF52RxMsg.command = rxdByte[2];
    if ((nRF52RxMsg.commandType == 1 && (nRF52RxMsg.length == 0x20 || nRF52RxMsg.length == 0x1E)) || (nRF52RxMsg.commandType == 2 && nRF52RxMsg.length < 0x55)) // DATA length is either 0x20 (RAW) or 0x1E (QUATERNIONS)
    {
      return 1;
    }
    else
    {
      return 0;
    }
  }
  else
  {
    /* Less than 2 elems in the buffer */
	return 0;
  }
}

static int nRF52_BodyPartPresent(void)
{
  if (ring_buffer_num_items(&nRF52RingbufRx) >= (nRF52RxMsg.length))
  {
    return 1;
  }
  else
  {
    return 0;
  }
}

static int nRF52_BuildBody(void)
{
  for (unsigned int i = 0; i < (nRF52RxMsg.length); i++)
  {
    if (!ring_buffer_peek(&nRF52RingbufRx, (char*)&nRF52RxMsg.DU[i], i))
    {
      /* No item at index... */
#if PRINTF_nRF52_COM
      sprintf(string,"%u [app_nRF52_com] [nRF52_BuildBody] ring_buffer_peek(), no item at index %02X\n",(unsigned int) HAL_GetTick(), nRF52RxMsg.DU[i]);
      xQueueSend(pPrintQueue, string, 0);
#endif
      return 0;
    }
    ring_buffer_peek(&nRF52RingbufRx, (char*)&nRF52RxMsg.CS, nRF52RxMsg.length-1);
  }
//#if PRINTF_nRF52_COM
//  sprintf(string,"%u [app_nRF52_com] [nRF52_BuildBody] Data Units of ring_buffer correctly transferred to nRF52RxMsg.DU\n",(unsigned int) HAL_GetTick());
//  xQueueSend(pPrintQueue, string, 0);
//#endif
  return 1;
}

static int nRF52_csCorrect(void)
{
  char CScalc = 0x00;
  for (unsigned int i = 0; i < (nRF52RxMsg.length-1); i++)
  {
    CScalc ^= nRF52RxMsg.DU[i];
  }
  ring_buffer_dequeue_arr(&nRF52RingbufRx, NULL, nRF52RxMsg.length);
  if (CScalc == nRF52RxMsg.CS)
  {
	return 1;
  }
  else
  {
    return 0;
  }
}

static int nRF52_TimedOut(unsigned int * var)
{
    *var = *var +1;
    if (*var >= 0x100)
    {
        *var = 0;
        return 1;
    }
    else
    {
        return 0;
    }
}

static void nRF52_RstTimeout(unsigned int * var)
{
    *var = 0;
}

//void rsv_data_handler(uint8_t * buf, uint8_t sensor_number, uint8_t data_format){
//
//	int16_t sd_card_buffer [100];
//	int16_t data [10];
//	uint16_t data_values;
//	uint32_t timestamp;
//	uint16_t pakket_send_nr;
//
//	switch(data_format){
//		case DATA_FORMAT_1: {	data_values = 4;		} break;
//		case DATA_FORMAT_2: {	data_values = 3;		} break;
//		case DATA_FORMAT_3: {	data_values = 3;		} break;
//		case DATA_FORMAT_4: {	data_values = 6;		} break;
//		case DATA_FORMAT_5: {	data_values = 10;		} break;
//	}
//
//	//int16_t sd_card_buffer [NUMBER_OF_DATA_READS_IN_nRF52_PACKET * data_values];
//
//
//  for(uint8_t j = 0; j < NUMBER_OF_nRF52_PACKETS; j++){
//    uint16_t start_pos = PACKET_START_POS + 7;
//		// 1e byte: 			command "IMU_SENSOR_MODULE_REQ_SEND_DATA"
//		// 2e & 3e byte:		packet_send_nr
//		// 3e 4e 5e 6e byte:	timestamp
//
//		pakket_send_nr = buf[PACKET_START_POS + 1] | (buf[PACKET_START_POS + 2] << 8);
//		timestamp = buf[PACKET_START_POS + 3] | (buf[PACKET_START_POS + 4] << 8) | (buf[PACKET_START_POS + 5] << 16) | (buf[PACKET_START_POS + 6] << 24);
//
//    for(int i = 0; i < NUMBER_OF_DATA_READS_IN_nRF52_PACKET; i++){
//      //int16_t data [data_values];
//			for(uint8_t g = 0; g < data_values; g++){
//				data[g] = ((buf[start_pos + g*2 + data_values*2 * i] << 8) | buf[start_pos + g*2 + 1 + data_values*2 * i]);
//			}
//
//			for(uint8_t k = 0; k < data_values; k++){
//				sd_card_buffer [j*NUMBER_OF_DATA_READS_IN_nRF52_PACKET + i*data_values + k] = data[k];
//			}
//    }
//  }
//
//	SD_CARD_COM_save_data(pakket_send_nr, timestamp, sensor_number, sd_card_buffer, data_values, data_format);
//	//SD_CARD_COM_save_data_qga(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
//	//SD_CARD_COM_save_data_q(pakket_send_nr, timestamp, sensor_number, sd_card_buffer);
//}

static void BLEmoduleDataToSensorEvent1(int32_t data[20], imu_100Hz_data_t *sensorEvent)
{
   sensorEvent->rotVectors1.real = data[0];
   sensorEvent->rotVectors1.i    = data[1];
   sensorEvent->rotVectors1.j    = data[2];
   sensorEvent->rotVectors1.k    = data[3];

   sensorEvent->gyroscope1.x     = data[7];
   sensorEvent->gyroscope1.y     = data[8];
   sensorEvent->gyroscope1.z     = data[9];

   sensorEvent->accelerometer1.x = data[4];
   sensorEvent->accelerometer1.y = data[5];
   sensorEvent->accelerometer1.z = data[6];

   sensorEvent->magnetometer1.x  = data[10];
   sensorEvent->magnetometer1.y  = data[11];
   sensorEvent->magnetometer1.z  = data[12];
}

static void BLEmoduleDataToSensorEvent2(int32_t data[20], imu_100Hz_data_t *sensorEvent)
{
   sensorEvent->rotVectors2.real = data[0];
   sensorEvent->rotVectors2.i    = data[1];
   sensorEvent->rotVectors2.j    = data[2];
   sensorEvent->rotVectors2.k    = data[3];

   sensorEvent->gyroscope2.x     = data[7];
   sensorEvent->gyroscope2.y     = data[8];
   sensorEvent->gyroscope2.z     = data[9];

   sensorEvent->accelerometer2.x = data[4];
   sensorEvent->accelerometer2.y = data[5];
   sensorEvent->accelerometer2.z = data[6];

   sensorEvent->magnetometer2.x  = data[10];
   sensorEvent->magnetometer2.y  = data[11];
   sensorEvent->magnetometer2.z  = data[12];
}

void get_battery_levels(BATTERY_ARRAY *batt)
{
	memcpy(batt, &battery, sizeof(*batt));
}
