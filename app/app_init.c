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
//#include "app_imu.h"
#include "app_BLEmodule1.h"
#include "app_BLEmodule2.h"
#include "app_BLEmodule3.h"
#include "app_BLEmodule4.h"
#include "app_BLEmodule5.h"
#include "app_BLEmodule6.h"

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

#include "../pinterface/interface_sd.h"
#include "../pinterface/pUART.h"
#include "../pinterface/pI2C.h"

#include "../board/board.h"
#include "common.h"
#include "data/structures.h"			/* Contains the data structures */
#include "app_terminal_com.h"
#include "app_BT1_com.h"
#include "../Inc/imu_com.h"
#include "../Inc/main.h"

#include "nRF52_driver.h"
#include "app_nRF52_com.h"

/** OLED **/
#include "ssd1306.h"

#define PRINTF_APP_INIT 1

static void initThread_init(void);
void initThread(const void * params);

/** OLED Thread **/
static osThreadId oledThreadHanlder;
static void initThread_init(void);
void oledThread(const void * params);

static osThreadId initThreadHanlder;

extern imu_module imu_1;
extern imu_module imu_2;
extern imu_module imu_3;
extern imu_module imu_4;
extern imu_module imu_5;
extern imu_module imu_6;
extern imu_module imu_7;
extern imu_module imu_8;

extern imu_module *imu_array [];
extern char string[];
extern QueueHandle_t pPrintQueue;

// for nrF52, contains all MAC addresses

dcu_conn_dev_t mac_addr[8];
int numberOfModules = 0; // this is the number of modules defined in the imu_arry (or RAW file).
int numberOfModulesMACAddressAvailable = 0;
int numberOfModulesConnected = 0;
int numberOfModulesCalibrated = 0;
int numberOfModulesSampleRateGiven = 0;
int numberOfModulesOutputDataTypeGiven = 0;
int numberOfModulesSynchronized = 0;

static void oledThread_init(void)
{
	osThreadDef(oledManager, oledThread, osPriorityLow, 0, 256);
	oledThreadHanlder = osThreadCreate (osThread(oledManager), NULL);
}

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
	UART7_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART7_Init_Protection() started (Terminal).\n");
	HAL_UART_Transmit(&huart7, (uint8_t *)string, strlen(string), 25);
#endif
	UART3_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART3_Init_Protection() started (Android device).\n");
	HAL_UART_Transmit(&huart7, (uint8_t *)string, strlen(string), 25);
#endif
	UART4_Init_Protection();
	HAL_Delay(100);
#if PRINTF_APP_INIT
	sprintf(string, "[APP_INIT] UART4_Init_Protection() started (nRF52).\n");
	HAL_UART_Transmit(&huart7, (uint8_t *)string, strlen(string), 25);
#endif

//	I2C_Init();
	/* Start the timer */
//	tim2_start();

	/** Thread initialization **/
	initThread_init();
//!	imu_task_init();
	BLEmodule1_task_init();
	BLEmodule2_task_init();
	BLEmodule3_task_init();
	BLEmodule4_task_init();
	BLEmodule5_task_init();
	BLEmodule6_task_init();
	//gps_init_task();
	cpl_init_rx_task();
	//PWC_Init_Thread();
	usbad_task_init();
	initStreamerThread();
	rtos_rtc_thread_init();
	//initCanPollerThread();
//	rtos_measurement_storage_init();
	initSyncThread();
	com_init_rx_task(); // communication via terminal
//	bt_init_rx_task();  // bluetooth initialisation (changed to nRF52 initialisation)
	nRF52_init_rx_task(); // nRF52 initialisation

	/** Queue initialization **/
	conf_service_queue_init();
	//can_msg_queue_init();
	streamer_service_queue_init();
	measurements_service_queue_init();
	android_com_queue_init();

	/** Init OLED Screen **/
	ssd1306_Init();
	ssd1306_nomade();

	/** Battery voltage measurements **/
	measBatt_Init();
	float battVoltage;
	measBatt(&battVoltage);

	ssd1306_battery();
	dcu_set_text_battery2(&battVoltage);

	osDelay(2000);
}

/* initThread
*
* @param nothing
* @return nothing
*/
void initThread(const void * params)
{

  osDelay(4000);
  comm_set_sync(COMM_CMD_START_SYNC);
  dcu_set_text_2_lines("Sync", "started");
  osDelay(1000);


  int rawFileRetrieved = 0;
  int onlyonce = 0;
  FATFS_Init(); /* Initialize file system and mount SD Card */
//#if PRINTF_APP_INIT
//  xQueueSend(pPrintQueue, "[app_init] [initThread] Try to retrieve RAW Configuration File.\n", 0); // print string via pPrintGatekeeperTask
//#endif
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
  memset(mac_addr, 0xFF, sizeof(mac_addr));
  for (int i = 0; i<8; i++)
  {
    if (imu_array[i]->macAddressAvailable != 0)
	{
      numberOfModules++;
      // Copy MAC address to mac_addr table
      for (int j = 5; j >= 0; j--)
      {
  	    mac_addr[i].addr[5-j] = imu_array[i]->mac_address[j];
      }
	}
  }
  if (numberOfModules > 1)
  {
#if PRINTF_APP_INIT
    sprintf(string, "%u [app_init] [initThread] %u MAC addresses found in RAW file. Send list to nRF52.\n",(unsigned int) HAL_GetTick(), (unsigned int)numberOfModules);
    xQueueSend(pPrintQueue, string, 0);
#endif
  }
  else
  {
  	if (numberOfModules == 1)
  	{
#if PRINTF_APP_INIT
      sprintf(string, "%u [app_init] [initThread] 1 MAC address found in RAW file. Send it to nRF52.\n",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
#endif
  	}
  	else
  	{
#if PRINTF_APP_INIT
      sprintf(string, "%u [app_init] [initThread] No BLE nodes defined in RAW file. Upload new RAW file.\n",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
#endif
  	}
  }
  for(;;)
  {
	osDelay(2000);
    if (rawFileRetrieved)
	{
      if (numberOfModules)
      {
        if (numberOfModulesMACAddressAvailable != numberOfModules)
        {

#if PRINTF_APP_INIT
      sprintf(string, "%u [app_init] [initThread] Send mac addresses.\n",(unsigned int) HAL_GetTick());
      xQueueSend(pPrintQueue, string, 0);
#endif

          comm_set_mac_addr(mac_addr,sizeof(mac_addr));
        }
        else
        {
          if (numberOfModulesConnected != numberOfModules)
          {
            for (int i = 0; i < numberOfModules; i++)
            {
      	      if(!imu_array[i]->measuring)
      	      {
          	    if (!imu_array[i]->connected)
           	    {
#if PRINTF_APP_INIT
                  sprintf(string, "[app_init] [initThread] BLE nodes defined, try to connect %s. Move module.\n",imu_array[i]->name);
                  xQueueSend(pPrintQueue, string, 0);


                  dcu_set_text_2_lines("Move","Modules");

#endif
          	    }
      	      }
            }
          }
          else
          {
            if (numberOfModulesCalibrated != numberOfModules)
            {

              for (int i = 0; i < numberOfModules; i++)
              {
                if(!imu_array[i]->measuring)
                {
            	   if (imu_array[i]->is_calibrated != COMM_CMD_CALIBRATION_DONE)
            	   {
            	     switch(imu_array[i]->is_calibrated)
                     {
            	       case COMM_CMD_CALIBRATION_START:
            	       {
#if PRINTF_APP_INIT
                         sprintf(string, "[app_init] [initThread] Try to calibrate gyroscope of connected %s. Do not move module.\n",imu_array[i]->name);
                         xQueueSend(pPrintQueue, string, 0);

                         dcu_set_text_cal_gyro();
#endif
            		     break;
            	       }
            	       case COMM_CMD_CALIBRATION_GYRO_DONE:
            	       {
#if PRINTF_APP_INIT
                         sprintf(string, "[app_init] [initThread] Try to calibrate accelerometer of connected %s. Place module in 3-axes, one by one.\n",imu_array[i]->name);
                         xQueueSend(pPrintQueue, string, 0);

                         dcu_set_text_cal_accel();
#endif
            		     break;
            	       }
            	       case COMM_CMD_CALIBRATION_ACCEL_DONE:
            	       {
#if PRINTF_APP_INIT
                         sprintf(string, "[app_init] [initThread] Try to calibrate magnetometer of connected %s. Move module in an 8 shape.\n",imu_array[i]->name);
                         xQueueSend(pPrintQueue, string, 0);

                         dcu_set_text_cal_mag();
#endif
            	         break;
            	       }
            	       case COMM_CMD_CALIBRATION_DONE:
            	       {
            	    	   dcu_set_text_2_lines("IMU","Calibrated");
            	    	   osDelay(2000);
            	       }
            		   default:
            		   {
#if PRINTF_APP_INIT
                         sprintf(string, "[app_init] [initThread] None defined value in imu_array[i]->is_calibrated.\n");
                         xQueueSend(pPrintQueue, string, 0);
#endif

            		   }
            	     }
            	   }
                 }
               }
             }
             else
             {
                if (numberOfModulesSampleRateGiven != numberOfModules)
                {
#if PRINTF_APP_INIT
                   sprintf(string, "%u [app_init] [initThread] Set sample frequency to %dHz of all modules.\n",(unsigned int) HAL_GetTick(),imu_array[0]->sampleFrequency);
                   xQueueSend(pPrintQueue, string, 0);

                   dcu_set_text_2_lines("Set sample", "frequency");
#endif
                   comm_set_frequency(imu_array[0]->sampleFrequency);
                 }
                 else
                 {
                	if (numberOfModulesOutputDataTypeGiven != numberOfModules)
                	{ // set output data type:
                		if(!imu_array[0]->outputDataTypeGiven)
                		{
#if PRINTF_APP_INIT
                           xQueueSend(pPrintQueue, "[app_init] [initThread] Set output data type of all calibrated modules ", 0);
#endif
                           uint8_t outputDataTypeValue = 0;
            	           switch(imu_array[0]->outputDataType)
            	           {
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT:
            	              {
            	                 outputDataTypeValue = COMM_CMD_MEAS_QUAT6;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to quaternions, 6DOF.\n", 0);

            	                 dcu_set_text_2_lines("Quat 6 DoF", "50 Hz");
#endif
            	                 break;
            	              }
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_100HZ:
            	              {
            	                 outputDataTypeValue = COMM_CMD_MEAS_QUAT6;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to quaternions, 6DOF.\n", 0);

            	                 dcu_set_text_2_lines("Quat 6 DoF", "100 Hz");
#endif
            	                 break;
            	              }
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG:
            	              {
            	    	         outputDataTypeValue = COMM_CMD_MEAS_RAW;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to RAW: gyroscope, accelerometer, magnetometer.\n", 0);

            	                 dcu_set_text_2_lines("RAW Data", "50 Hz");
#endif
            	    	         break;
            	              }
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUGYRO_ACC_MAG_100HZ:
            	              {
            	    	         outputDataTypeValue = COMM_CMD_MEAS_RAW;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to RAW: gyroscope, accelerometer, magnetometer.\n", 0);

            	                 dcu_set_text_2_lines("RAW Data", "100 Hz");
#endif
            	    	         break;
            	              }
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF:
            	              {
            	    	         outputDataTypeValue = COMM_CMD_MEAS_QUAT9;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to quaternions, 9DOF.\n", 0);

            	                 dcu_set_text_2_lines("Quat 9 DoF", "50 Hz");
#endif
            	    	         break;
            	              }
            	              case SETUP_PRM_DATA_OUTPUT_DATATYPE_IMUQUAT_9DOF_100HZ:
            	              {
            	    	         outputDataTypeValue = COMM_CMD_MEAS_QUAT9;
#if PRINTF_APP_INIT
            	                 xQueueSend(pPrintQueue, "to quaternions, 9DOF.\n", 0);

            	                 dcu_set_text_2_lines("Quat 9 DoF", "100 Hz");
#endif
            	    	         break;
            	              }
            	              default:
            	              {
             	    	         outputDataTypeValue = COMM_CMD_MEAS_RAW;
#if PRINTF_APP_INIT
                                 xQueueSend(pPrintQueue, "to RAW: gyroscope, accelerometer, magnetometer. But undefined value in RAW file.\n", 0);

                                 dcu_set_text_2_lines("Default setting", "(RAW)");
#endif
              	                 break;
            	              }
            	           }
            	           comm_set_data_type(outputDataTypeValue);
            	           //osDelay(8000);
            	        }
                	}
                	else
                	{
//                    	if (numberOfModulesSynchronized != numberOfModules)
//                    	{ // synchronize connected modules:
#if PRINTF_APP_INIT
                           //xQueueSend(pPrintQueue, "[app_init] [initThread] Start module synchronization.\n", 0);
#endif
             	           // comm_set_sync(COMM_CMD_START_SYNC);
             	           // dcu_set_text_2_lines("Sync", "started");

//             	           osDelay(8000);

//                    	}
//                    	else
//                    	{ // now module measurements can start:
#if PRINTF_APP_INIT
                           //xQueueSend(pPrintQueue, "[app_init] [initThread] Start module measurement.\n", 0);
#endif


             	          osDelay(4000);


             	          dcu_set_text_2_lines("Connect", "Tablet");

             	          // Wait until tablet connection
             	          while(tablet_get_time_received() == 0);
             	          tablet_set_time_received(0);

						  stm32_datetime_t datetime;
						  memset(&datetime, 0, sizeof(stm32_datetime_t));

						  // Get time from RTC in milliseconds
						  stm32_time_t epochNow_Ms;
						  app_rtc_get_unix_epoch_ms(&epochNow_Ms); /* Get the epoch unix time from the RTC */
						  // Convert to seconds
						  uint64_t epoch_AD_s = epochNow_Ms/1000;
						  // Convert to datetime struct
						  datetime = *localtime(&epoch_AD_s);

#if PRINTF_APP_INIT
						  strftime(string, 150, "[APP_INIT] %A %c", &datetime);
						  //sprintf(string, "[APP_INIT] h %d m %d s %d - day %d month %d year %d\n",
							//	  datetime.tm_hour, datetime.tm_min, datetime.tm_sec, datetime.tm_mday, datetime.tm_mday, datetime.tm_year);
						  xQueueSend(pPrintQueue, string, 0);
#endif


						  // Start measurement
						  comm_start_meas_w_time(&datetime);

						   /** From now on, start displaying that measurement is going on **/
             	           oledThread_init();

             	           osDelay(1000);
              	           osThreadTerminate(initThreadHanlder);
//                    	}
                	}
                }
       		 }
           }
         }
      }
	}
  }
}

void oledThread(const void * params)
{
	dcu_set_text_2_lines("Measurement", "Started");
	osDelay(2000);

	// Request battery level
	// comm_req_batt_lvl();
	// @Sarah I don't know how to handle the received data


	ssd1306_battery();
	float voltages[6];
	for(uint8_t i = 0; i<6; i++)
	{
		voltages[i] = 3.75f;
	}
	dcu_set_text_battery_sensors(&voltages[0], &voltages[1], &voltages[2], &voltages[3], &voltages[4], &voltages[5]);
	osDelay(10000);

	uint8_t x_start = 0;
	uint8_t x_len = 50;
	uint8_t x_end = x_start + x_len;

	uint8_t y_start = SSD1306_HEIGHT-10;
	uint8_t y_end = SSD1306_HEIGHT-1;

	bool forward_backward = 0;

	uint8_t step = 10;

	/** TODO: this while loop must be interrupted when measurements stopped **/
	while(1)
	{
		osDelay(500);

		ssd1306_Fill(Black);
		dcu_set_text_2_lines("Measurement", "Running");
		ssd1306_DrawRectangle(x_start, y_start, x_end, y_end, White);
		ssd1306_UpdateScreen();

		// Edge conditions
		if(x_end >= SSD1306_WIDTH-8)
		{
			forward_backward = 1;
		}else if(x_start <= 0)
		{
			forward_backward = 0;
		}

		if(forward_backward)
		{
			x_start-=step;
			x_end-=step;
		}else{
			x_start+=step;
			x_end+=step;
		}
	}
}
