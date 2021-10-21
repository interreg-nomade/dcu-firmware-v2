/**
 * @file app_init.c
 * @brief GPS Application file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date April 2019
 * Refactor in August 2019
 *
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 *
 * Contains the initialisation part of the project - pre-RTOS running inits, init & monitoring task
 */

/* Standard libraries */
#include <string.h>
#include <time.h>

/* Own header */
#include "app_init.h"
#include "app_imu.h"

#include "app_rtc.h"
#include "app_tablet_com.h"

#include "app_streamer.h"
//#include "app_rtc.h"


#include "app_usbad.h"
#include "app_storage.h"
#include "app_sync.h"


#include "queues/streamer_service/streamer_service_queue.h"
#include "queues/measurements/measurements_queue.h"
#include "queues/android_device_com/android_com_queue.h"

#include "interface_sd.h"
#include "pUART.h"
#include "pI2C.h"

#include "board.h"
#include "common.h"
#include "data/structures.h"			/* Contains the data structures */
#include "app_terminal_com.h"
#include "app_BT1_com.h"
#include "../../Inc/imu_com.h"
#include "../../Inc/main.h"

#define PRINTF_APP_INIT 1

static void initThread_init(void);
void initThread(const void * params);

static osThreadId initThreadHanlder;

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;

extern imu_module *imu_array [];
extern char string[];
extern QueueHandle_t pPrintQueue;

static void initThread_init(void)
{
	osThreadDef(configManager, initThread, osPriorityHigh, 0, 256);
	initThreadHanlder = osThreadCreate (osThread(configManager), NULL);
}

void project_init(void)
{
	/* Initialization of the common configuration */
	common_config_init();
	HAL_Delay(10);
	/******* Init the INTERFACES ******/
	UART5_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART5_Init_Protection() started (Terminal).\n");
	HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
#endif
	UART3_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART3_Init_Protection() started (Android device).\n");
	HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
#endif
	UART4_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART4_Init_Protection() started (BT1).\n");
	HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
#endif

//	I2C_Init();
	/* Start the timer */
//	tim2_start();

	/** Thread initialization **/
	initThread_init();
	imu_task_init();
	//gps_init_task();
	cpl_init_rx_task();
	//PWC_Init_Thread();
	usbad_task_init();
	initStreamerThread();
	rtos_rtc_thread_init();
	//initCanPollerThread();
	rtos_measurement_storage_init();
	initSyncThread();
	com_init_rx_task(); // communication via terminal
	bt_init_rx_task();  // bluetooth initialisation




	/** Queue initialization **/
	conf_service_queue_init();
	//can_msg_queue_init();
	streamer_service_queue_init();
	measurements_service_queue_init();
	android_com_queue_init();
}

/* initThread
*
* @param nothing
* @return nothing
*/
void initThread(const void * params)
{
  int rawFileRetrieved = 0;
  int BTArrayNotEmpty = 0;
  FATFS_Init(); /* Initialize file system and mount SD Card */
#if PRINTF_APP_INIT
  xQueueSend(pPrintQueue, "[app_init] [initThread] Try to retrieve RAW Configuration File.\n", 0); // print string via pPrintGatekeeperTask
#endif
  rawFileRetrieved = rawConfFile_RetrieveConf(); /* Retrieve raw configuration if there is one */
#if PRINTF_APP_INIT
  if (rawFileRetrieved)
  {
	xQueueSend(pPrintQueue, "[app_init] [initThread] RAW File Retrieved.\n", 0);
  }
  else
  {
	xQueueSend(pPrintQueue, "[app_init] [initThread] Error retrieving RAW File.\n", 0);
  }
#endif
  osDelay(100);
  for (int i = 0; i<6; i++)
  {
    if (imu_array[i]->macAddressAvailable != 0)
	{
#if PRINTF_APP_INIT
      char DUString[4];
      sprintf(string, "[app_init] [initThread] Available MAC Addresses for %s: ",imu_array[i]->name);
      for (int j = 5; j >= 0; j--)
      {
        if (strlen(string) < 147)
        {
  	   	  sprintf(DUString, "%02X.",imu_array[i]->mac_address[j]);
  	   	  strcat(string, DUString);
        }
      }
  	  strcat(string,"\n");
      xQueueSend(pPrintQueue, string, 0);
#endif
	  BTArrayNotEmpty = 1;
	}
  }
  for(;;)
  {
    if (rawFileRetrieved)
	{
      if (BTArrayNotEmpty)
      {
        for (int i = 0; i < NUMBER_OF_SENSOR_SLOTS; i++)
        {
    	  if(!imu_array[i]->measuring)
    	  {
            if (imu_array[i]->macAddressAvailable != 0)
            {
        	  if (!imu_array[i]->connected)
        	  {
#if PRINTF_APP_INIT
                sprintf(string, "[app_init] [initThread] Bluetooth nodes defined, try to connect %s. Switch on module.\n",imu_array[i]->name);
                xQueueSend(pPrintQueue, string, 0);
#endif
                BT_connect(imu_array[i]->uart, imu_array[i]->mac_address);
        	  }
        	  else
        	  { // module connected
        	    HAL_GPIO_WritePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin, GPIO_PIN_RESET);
      		    HAL_GPIO_WritePin(LED_BUSY_GPIO_Port, LED_BUSY_Pin, GPIO_PIN_SET);
        	    if(!imu_array[i]->is_calibrated)
        	    {
#if PRINTF_APP_INIT
                  sprintf(string, "[app_init] [initThread] Try to calibrate connected %s. Do not move module.\n",imu_array[i]->name);
                  xQueueSend(pPrintQueue, string, 0);
#endif
            	  BT_transmitMsg_CMD(imu_array[i]->uart, IMU_SENSOR_MODULE_REQ_START_CALIBRATION);
        	    }
        	    else
        	    { // module connected & calibrated
        		  if(!imu_array[i]->sampleRateGiven)
        		  {
#if PRINTF_APP_INIT
                    sprintf(string, "[app_init] [initThread] Set sample frequency to %dHz of calibrated %s.\n",imu_array[i]->sampleFrequency,imu_array[i]->name);
                    xQueueSend(pPrintQueue, string, 0);
#endif
                    uint8_t value = 0;
            	    switch(imu_array[i]->sampleFrequency)
            	    {
            		  case 10:  value = SAMPLING_FREQ_10HZ;  break;
            		  case 20:  value = SAMPLING_FREQ_20HZ;  break;
            		  case 25:  value = SAMPLING_FREQ_25HZ;  break;
            		  case 50:  value = SAMPLING_FREQ_50HZ;  break;
            		  case 100: value = SAMPLING_FREQ_100HZ; break;
            		  default:
            		  {
            		    value = SAMPLING_FREQ_50HZ;
#if PRINTF_APP_INIT
                        sprintf(string, "[app_init] [initThread] Set sample frequency to 50Hz of calibrated %s, because wrong value was given in RAW file.\n",imu_array[i]->name);
                        xQueueSend(pPrintQueue, string, 0);
#endif
            		  }
            	    }
                    if (value != 0)
                    {
                      BT_transmitMsg_CMD_Data(imu_array[i]->uart, IMU_SENSOR_MODULE_REQ_SAMPLING_FREQ_CHANGE, 1, &value);
                    }
        		  }
        		  else
        		  { // module connected, calibrated & sample rate set
            	    if(!imu_array[i]->outputDataTypeGiven)
            	    {
#if PRINTF_APP_INIT
                      sprintf(string, "[app_init] [initThread] Set output data type of calibrated %s ",imu_array[i]->name);
                      xQueueSend(pPrintQueue, string, 0);
#endif
                      uint8_t outputDataTypeValue = 0;
            	      switch(imu_array[i]->outputDataType)
            	      {
            	        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
            	        {
            	          outputDataTypeValue = DATA_FORMAT_1;
#if PRINTF_APP_INIT
            	          xQueueSend(pPrintQueue, "to quaternions only.\n", 0);
#endif
            	          break;
            	        }
            	        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUATBAT:
            	        { // the battery voltage is not yet implemented
            	          outputDataTypeValue = DATA_FORMAT_1;
#if PRINTF_APP_INIT
            	          xQueueSend(pPrintQueue, "to quaternions only.\n", 0);
#endif
            	          break;
            	        }
            	        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC:
            	        {
            	    	  outputDataTypeValue = DATA_FORMAT_5;
#if PRINTF_APP_INIT
            	          xQueueSend(pPrintQueue, "to quaternions, gyroscope and accelerometer.\n", 0);
#endif
            	    	  break;
            	        }
            	        case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_GYRO_ACC_100HZ:
            	        {
            	    	  outputDataTypeValue = DATA_FORMAT_5;
#if PRINTF_APP_INIT
            	          xQueueSend(pPrintQueue, "to quaternions, gyroscope and accelerometer @ 100Hz.\n", 0);
#endif
            	    	  break;
            	        }
            	        default:
            	        {
              	          outputDataTypeValue = DATA_FORMAT_1;
#if PRINTF_APP_INIT
                          sprintf(string, "to quaternions only. But undefined value in RAW file: %08X.\n",(int)imu_array[i]->outputDataType);
                          xQueueSend(pPrintQueue, string, 0);
#endif
              	          break;
            	        }
            	      }
            	      BT_transmitMsg_CMD_Data(imu_array[i]->uart, IMU_SENSOR_MODULE_REQ_CHANGE_DF, 1, &outputDataTypeValue);
            	    }
            	    else
            	    { // module connected, calibrated, sample rate set and output data type set
              		  HAL_GPIO_WritePin(LED_BUSY_GPIO_Port, LED_BUSY_Pin, GPIO_PIN_RESET);
            		  HAL_GPIO_WritePin(LED_GOOD_GPIO_Port, LED_GOOD_Pin, GPIO_PIN_SET);
                      IMU_start_measurements_without_sync((void *)imu_array[i]); /* send message to BT1 to start the IMU measurement */
                      imu_array[i]->measuring = 1;
            		}
            	  }
        		}
        	  }
        	}
          }
        }
      }
      else
      {
    	HAL_GPIO_TogglePin(LED_ERROR_GPIO_Port, LED_ERROR_Pin);
#if PRINTF_APP_INIT
        xQueueSend(pPrintQueue, "[app_init] [initThread] No Bluetooth nodes defined in RAW file. Upload new RAW file.\n", 0);
#endif
      }
	}
	osDelay(4000);
  }
}
