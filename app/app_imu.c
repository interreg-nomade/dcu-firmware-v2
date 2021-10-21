/**
 * @file app_imu.c
 * @brief GPS Application file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date April 2019
 * Refactor in August 2019
 *
 * Contains the GPS application thread - get data, decode, and push it into the real-time data structure
 */

#include <stdio.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "cmsis_os.h"
#include "usart.h"

#include "app_imu.h"
#include "bno080-driver/sh2.h"
#include "bno080-driver/sh2_err.h"
#include "bno080-driver/sh2_SensorValue.h"

// Sensor Application
#include "common.h"

#include "project_config.h"


#define USING_CONF_IMU 				1
#define IMU_USING_EXTERNAL			0
#define IMU_USING_INTERNAL 			1

#define PRINTF_APP_IMU_DBG			0

#if PRINT_APP_IMU_DBG_MSG
#define PRINT_APP_IMU_ACCL			0
#define PRINT_APP_IMU_GYRO			0
#define PRINT_APP_IMU_MAG			0
#endif

#define SENSOR_EVENT_QUEUE_SIZE (30) //Size of the queue which will handle the sensor value, if sample frequency is set to 10Hz, this could be 100

//#define DSF_OUTPUT // Define this to produce DSF data for logging
// #define PERFORM_DFU // Define this to perform firmware update at startup.
// #define CONFIGURE_HMD // Define this to use HMD-appropriate configuration
// #define CONFIGURE_RVC // Define this for calibration config

#ifndef ARRAY_LEN
#define ARRAY_LEN(a) (sizeof(a)/sizeof(a[0]))
#endif
#define FIX_Q(n, x) ((int32_t)(x * (float)(1 << n)))

const float scaleDegToRad = 3.14159265358 / 180.0;

#ifdef PERFORM_DFU
#include "dfu_bno080.h"
#include "firmware.h"
#endif

static void eventHandler(void * cookie, sh2_AsyncEvent_t *pEvent);
//!static void sensorHandler(void * cookie, sh2_SensorEvent_t *pEvent);
static void onReset(void);

#ifdef DSF_OUTPUT
static void printDsfHeaders(void);
static void printDsf(const sh2_SensorEvent_t * event);
#endif

#ifndef DSF_OUTPUT
static void reportProdIds(void);
#endif

static void imuTask(const void * params);
static void imu_init(void);
//static void refreshImuData(const sh2_SensorEvent_t *pEvent, imu_data_t * imu);
static void refreshImuData(const imu_100Hz_data_t *sensorEvent, imu_100Hz_data_t * imu);
void IMU_Config_Init();
int IMU_Config_Still_Valid(void);


sh2_ProductIds_t prodIds;
SemaphoreHandle_t wakeSensorTask;
volatile bool resetPerformed = false;
volatile bool startedReports = false;
QueueHandle_t eventQueue;

static osThreadId imuTaskHandle;
static instrument_config_t * pImuInstrument;

extern void sh2_hal_init(void);
extern char string[];
extern QueueHandle_t pPrintQueue;


void imu_task_init()
{
  osThreadDef(imuTask01, imuTask, osPriorityAboveNormal, 0, 1024);	/* Declaration of IMU task */
  imuTaskHandle = osThreadCreate(osThread(imuTask01), NULL);			/* Start IMU task */
}

static void imuTask(const void * params)
{
	static uint32_t measurementCounter = 0;    //Measurement counter
//!	sh2_SensorEvent_t sensorEvent;	//Sensor Event (new data)
	imu_100Hz_data_t sensorEvent;
	imu_100Hz_data_t resImuData;		//Structure to store the IMU data

	const unsigned int streamPeriod = 20; // 20ms -> if this is being changed, change also localCounter in timer_callback.c
	unsigned int previousQueuing = 0; //Store the time of the previous post in main queue

	imu_init(); //Initialization of the IMU, RTOS resources and SH2 layer.

	for(;;)
	{
		// Wait until something happens
//		xSemaphoreTake(wakeSensorTask, portMAX_DELAY);

//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [imuTask] wakeSensorTask semaphore taken.\n", (unsigned int) HAL_GetTick());
//	      xQueueSend(pPrintQueue, string, 0);
//#endif

		// Dequeue sensor events
//		while (xQueueReceive(eventQueue, &sensorEvent, 0) == pdPASS)
//		{

//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [imuTask] Received sensorEvent from eventQueue.\n", (unsigned int) HAL_GetTick());
//	      xQueueSend(pPrintQueue, string, 0);
//#endif
//
//			measurementCounter++;
//#ifdef DSF_OUTPUT
//			printDsf(&sensorEvent);
//#else
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [imuTask] Received sensorEvent data: real= %f, i= %f, j= %f, k= %f.\n",
//				(unsigned int) HAL_GetTick(),
//				sensorEvent.rotVectors.real,
//				sensorEvent.rotVectors.i,
//				sensorEvent.rotVectors.j,
//				sensorEvent.rotVectors.k);
//	      xQueueSend(pPrintQueue, string, 0);
//#endif
//			//printEvent(&sensorEvent);
//			refreshImuData(&sensorEvent, &resImuData);
//#endif
//		}
#if USING_CONF_IMU
		/* Previous message have been acknowledged */
		/* It is potentially time to send a new message */
		if (xTaskGetTickCount() >= (previousQueuing + streamPeriod))
		{
			xSemaphoreTake(wakeSensorTask, portMAX_DELAY);
			xQueueReceive(eventQueue, &sensorEvent, 0);
			measurementCounter++;
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [imuTask] Received sensorEvent data: %04X %04X %04X %04X - %04X %04X %04X - %04X %04X %04X\n",
				(unsigned int) HAL_GetTick(),
				(unsigned int) sensorEvent.rotVectors1.real,
				(unsigned int) sensorEvent.rotVectors1.i,
				(unsigned int) sensorEvent.rotVectors1.j,
				(unsigned int) sensorEvent.rotVectors1.k,
				(unsigned int) sensorEvent.gyroscope1.x,
				(unsigned int) sensorEvent.gyroscope1.y,
				(unsigned int) sensorEvent.gyroscope1.z,
				(unsigned int) sensorEvent.accelerometer1.x,
				(unsigned int) sensorEvent.accelerometer1.y,
				(unsigned int) sensorEvent.accelerometer1.z);
	    xQueueSend(pPrintQueue, string, 0);
#endif


//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [imuTask] Received sensorEvent data: real= %f, i= %f, j= %f, k= %f.\n",
//				(unsigned int) HAL_GetTick(),
//				sensorEvent.rotVectors.real,
//				sensorEvent.rotVectors.i,
//				sensorEvent.rotVectors.j,
//				sensorEvent.rotVectors.k);
//	    xQueueSend(pPrintQueue, string, 0);
//#endif
			refreshImuData(&sensorEvent, &resImuData);

			/* Copy the actual  IMU data to the structure that will be queued by reference */
			/* Time to stream a message */
			if (pImuInstrument) //TODO: Logic AND config still correct
			{
				imu_100Hz_data_t * pData 	= NULL;
				pData 				= (imu_100Hz_data_t*) pImuInstrument->data;
				*pData 				= resImuData;
#if 0
				printf("\e[1;1H\e[2J");
				printf("ax : %.2f\n", resImuData.accelerometer.x);
				printf("ay : %.2f\n", resImuData.accelerometer.y);
				printf("az : %.2f\n", resImuData.accelerometer.z);
				printf("gx : %.2f\n", resImuData.gyroscope.x);
				printf("gy : %.2f\n", resImuData.gyroscope.y);
				printf("gz : %.2f\n", resImuData.gyroscope.z);
				printf("mx : %.2f\n", resImuData.magnetometer.x);
				printf("my : %.2f\n", resImuData.magnetometer.y);
				printf("mz : %.2f\n", resImuData.magnetometer.z);
#endif
			}
			previousQueuing = xTaskGetTickCount();
		}

#endif
		if (resetPerformed)
		{
//!			onReset();
		}
	}
}

static void imu_init(void)
{
#if USING_CONF_IMU
  IMU_Config_Init(); // Wait the configuration to be decoded and link the pointer handler.
#endif

  wakeSensorTask = xSemaphoreCreateBinary(); // Creating binary semaphore
//	eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(sh2_SensorEvent_t)); // Creating a queue used to store the data coming from the IMU instrument
  eventQueue = xQueueCreate(SENSOR_EVENT_QUEUE_SIZE, sizeof(imu_100Hz_data_t)); // Creating a queue used to store the data coming from the IMU instrument

#if PRINTF_APP_IMU_DBG
  if (eventQueue == NULL)
  {
	xQueueSend(pPrintQueue, "[APP_IMU] [imu_init] Error creating IMU sensor event queue.\n", 0);
  }
  else
  {
	xQueueSend(pPrintQueue, "[APP_IMU] [imu_init] IMU sensor event queue created successfully.\n", 0);
  }
#endif

//#ifdef PERFORM_DFU
//	// Perform DFU
//	printf("[IMU] Starting DFU process\n");
//	int status = dfu(&firmware);
//
//	printf("[IMU] DFU completed with status: %d\n", status);
//
//	if (status == SH2_OK) {
//		// DFU Succeeded.  Need to pause a bit to let flash writes complete
//		vTaskDelay(10);  // 10ms pause
//	}
//#endif
//
//	resetPerformed = false;
//	startedReports = false;
//
//	// init SH2 layer
//	sh2_initialize(eventHandler, NULL);
//
//	// Register event listener
//	sh2_setSensorCallback(sensorHandler, NULL);
//
//	// wait for reset notification, or just go ahead after 100ms
//	int waited = 0;
//	while (!resetPerformed && (waited < 2000))
//	{
//		osDelay(1);
//		waited++;
//	}
//#ifdef DSF_OUTPUT
//	// Print DSF file headers
//	printDsfHeaders();
//#else
//	// Read and display BNO080 product ids
//	reportProdIds();
//#endif
//	// Perform on-reset setup of BNO080
//	onReset();
}

// --- Private methods ----------------------------------------------

static void eventHandler(void * cookie, sh2_AsyncEvent_t *pEvent)
{
	if (pEvent->eventId == SH2_RESET)
	{
		resetPerformed = true;
		xSemaphoreGive(wakeSensorTask);
	}
}

void sensorHandler(imu_100Hz_data_t *sensorEvent)
{
	xQueueSend(eventQueue, sensorEvent, 0);
	xSemaphoreGive(wakeSensorTask);
}

//static void sensorHandler(void * cookie, sh2_SensorEvent_t *pEvent)
//{
//	xQueueSend(eventQueue, pEvent, 0);
//	xSemaphoreGive(wakeSensorTask);
//}

#define GIRV_REF_6AG  (0x0207)  					   // 6 axis Game Rotation Vector
#define GIRV_REF_9AGM (0x0204) 						   // 9 axis Absolute Rotation Vector
#define GIRV_SYNC_INTERVAL (20000)                     // sync interval: 10000 uS (100Hz)
#define GIRV_MAX_ERR FIX_Q(29, (30.0 * scaleDegToRad)) // max error: 30 degrees
#define GIRV_ALPHA FIX_Q(20, 0.303072543909142)        // pred param alpha
#define GIRV_BETA  FIX_Q(20, 0.113295896384921)        // pred param beta
#define GIRV_GAMMA FIX_Q(20, 0.002776219713054)        // pred param gamma

#ifdef CONFIGURE_HMD
// Enable GIRV prediction for 28ms with 100Hz sync
#define GIRV_PRED_AMT FIX_Q(10, 0.028)             // prediction amt: 28ms
#else
// Disable GIRV prediction
#define GIRV_PRED_AMT FIX_Q(10, 0.0)               // prediction amt: 0
#endif


static void configure(void)
{
	int status = SH2_OK;
	uint32_t config[7];

	// Note: The call to sh2_setFrs below updates a non-volatile FRS record
	// so it will remain in effect even after the sensor hub reboots.  It's not strictly
	// necessary to repeat this step every time the system starts up as we are doing
	// in this example code.

	// Configure prediction parameters for Gyro-Integrated Rotation Vector.
	// See section 4.3.24 of the SH-2 Reference Manual for a full explanation.
	// ...
	config[0] = GIRV_REF_9AGM;           	  // Reference Data Type
	config[1] = (uint32_t)GIRV_SYNC_INTERVAL; // Synchronization Interval
	config[2] = (uint32_t)GIRV_MAX_ERR;       // Maximum error
	config[3] = (uint32_t)GIRV_PRED_AMT;      // Prediction Amount
	config[4] = (uint32_t)GIRV_ALPHA;         // Alpha
	config[5] = (uint32_t)GIRV_BETA;          // Beta
	config[6] = (uint32_t)GIRV_GAMMA;         // Gamma

	status = sh2_setFrs(GYRO_INTEGRATED_RV_CONFIG,
			config,
			ARRAY_LEN(config));

	if (status != SH2_OK) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [configure] Error: %d, from sh2_setFrs() in configure().\n", (unsigned int) HAL_GetTick(), status);
	    xQueueSend(pPrintQueue, string, 0);
#endif

	}

	// The sh2_setCalConfig does not update non-volatile storage.  This
	// only remains in effect until the sensor hub reboots.

#ifdef CONFIGURE_RVC
	// Enable planar calibration mode, which is designed for RVC applications
	status = sh2_setCalConfig(SH2_CAL_PLANAR);
#else
	// Enable dynamic calibration for A, G and M sensors
	status = sh2_setCalConfig(SH2_CAL_ACCEL | SH2_CAL_GYRO | SH2_CAL_MAG);
#endif
	if (status != SH2_OK) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [configure] Error: %d, from sh2_setCalConfig() in configure().\n", (unsigned int) HAL_GetTick(), status);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}
}

static void startReports(void)
{
	static sh2_SensorConfig_t config;
	int status;
	int sensorId;

	/* Enable rotational vector */
	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.alwaysOnEnabled = false;
	config.changeSensitivity = 0;
	//config.reportInterval_us = 40000;  // microseconds (25Hz)
	config.reportInterval_us = 20000; // microseconds (50 Hz)
	//config.reportInterval_us = 10000;  // microseconds (100Hz)
	//config.reportInterval_us = 1000;   // microseconds (1000Hz)
	config.batchInterval_us = 0;
	sensorId = SH2_ROTATION_VECTOR;
	status = sh2_setSensorConfig(sensorId, &config);
	if (status != 0) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [startReports] Error while enabling sensor %d.\n", (unsigned int) HAL_GetTick(), sensorId);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}

	/* Enable accelerometer */
	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.alwaysOnEnabled = false;
	config.changeSensitivity = 0;
	//config.reportInterval_us = 40000;  // microseconds (25Hz)
	config.reportInterval_us = 20000; // microseconds (50 Hz)
	//config.reportInterval_us = 10000;  // microseconds (100Hz)
	//config.reportInterval_us = 1000;   // microseconds (1000Hz)
	config.batchInterval_us = 0;

	sensorId = SH2_ACCELEROMETER;
	status = sh2_setSensorConfig(sensorId, &config);
	if (status != 0) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [startReports] Error while enabling sensor %d.\n", (unsigned int) HAL_GetTick(), sensorId);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}

	/* Enable gyroscope */
	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.alwaysOnEnabled = false;
	config.changeSensitivity = 0;
	//config.reportInterval_us = 40000;  // microseconds (25Hz)
	config.reportInterval_us = 20000; // microseconds (50 Hz)
	//config.reportInterval_us = 10000;  // microseconds (100Hz)
	//config.reportInterval_us = 1000;   // microseconds (1000Hz)
	config.batchInterval_us = 0;

	sensorId = SH2_GYROSCOPE_CALIBRATED;
	status = sh2_setSensorConfig(sensorId, &config);
	if (status != 0) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [startReports] Error while enabling sensor %d.\n", (unsigned int) HAL_GetTick(), sensorId);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}

	/* Enable magnetometer */
	config.changeSensitivityEnabled = false;
	config.wakeupEnabled = false;
	config.changeSensitivityRelative = false;
	config.alwaysOnEnabled = false;
	config.changeSensitivity = 0;
	config.reportInterval_us = 100000;  // microseconds (10Hz)
	// config.reportInterval_us = 1000;   // microseconds (1000Hz)
	config.batchInterval_us = 0;

	sensorId = SH2_MAGNETIC_FIELD_CALIBRATED;
	status = sh2_setSensorConfig(sensorId, &config);
	if (status != 0) {
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [startReports] Error while enabling sensor %d.\n", (unsigned int) HAL_GetTick(), sensorId);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}
}

static void onReset(void)
{
	// Configure calibration config as we want it
	configure();

	// Start the flow of sensor reports
	startReports();

	/* Tare the IMU */
	sh2_setTareNow(SH2_TARE_X | SH2_TARE_Y | SH2_TARE_Z,
			SH2_TARE_BASIS_ROTATION_VECTOR);


	// Toggle reset flag back to false
	resetPerformed = false;
}


#ifndef DSF_OUTPUT
static void reportProdIds(void)
{
	int status;

	memset(&prodIds, 0, sizeof(prodIds));
	status = sh2_getProdIds(&prodIds);

	if (status < 0) {
#ifdef IMU_DEMO_TASK_STATUS_PRINTF_ENABLED
#endif
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [reportProdIds] Error from sh2_getProdIds.\n", (unsigned int) HAL_GetTick());
	    xQueueSend(pPrintQueue, string, 0);
#endif
		return;
	}

	// Report the results
	for (int n = 0; n < prodIds.numEntries; n++) {
#ifdef IMU_DEMO_TASK_STATUS_PRINTF_ENABLED
#endif
#if PRINTF_APP_IMU_DBG
		sprintf(string, "%u [APP_IMU] [reportProdIds] Part %d : Version %d.%d.%d Build %d.\n",
				(unsigned int) HAL_GetTick(),
				prodIds.entry[n].swPartNumber,
				prodIds.entry[n].swVersionMajor, prodIds.entry[n].swVersionMinor,
				prodIds.entry[n].swVersionPatch, prodIds.entry[n].swBuildNumber);
	    xQueueSend(pPrintQueue, string, 0);
#endif
	}
}

static void refreshImuData(const imu_100Hz_data_t *sensorEvent, imu_100Hz_data_t * imu)
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

    //#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] parsed data: real= %f, i= %f, j= %f, k= %f.\n",
//				(unsigned int) HAL_GetTick(),
//				imu->rotVectors.real,
//				imu->rotVectors.i,
//				imu->rotVectors.j,
//				imu->rotVectors.k);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] parsed data: 0x%04X 0x%04X 0x%04X 0x%04X.\n",
//				(unsigned int) HAL_GetTick(),
//				imu->rotVectors.real,
//				imu->rotVectors.i,
//				imu->rotVectors.j,
//				imu->rotVectors.k);
//	    xQueueSend(pPrintQueue, string, 0);
//#endif

}


//static void refreshImuData(const sh2_SensorEvent_t *event, imu_data_t * imu)
//{
//	int rc;
//	sh2_SensorValue_t value;
//
//	float r, i, j, k, acc_deg, x, y, z;
//	float t;
//	unsigned int t_ms;
//	static int skip = 0;
//
//	rc = sh2_decodeSensorEvent(&value, event);
//	if (rc != SH2_OK) {
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] Error decoding sensor event: %d.\n", (unsigned int) HAL_GetTick(), rc);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//		return;
//	}
//
//	t     = value.timestamp / 1000000.0;  // time in seconds.
//	t_ms = (unsigned int) (value.timestamp / 1000.0); //time in milliseconds in an unsigned int
//
//	switch (value.sensorId) {
//	case SH2_MAGNETIC_FIELD_CALIBRATED:
//		imu->magnetometer.timestamp = t_ms;
//		imu->magnetometer.x = value.un.magneticField.x;
//		imu->magnetometer.y = value.un.magneticField.y;
//		imu->magnetometer.z = value.un.magneticField.z;
//#if PRINT_APP_IMU_MAG
//		printf("[IMU] Magneto:%.2f, %.2f, %.2f\n",
//				value.un.magneticField.x,
//				value.un.magneticField.y,
//				value.un.magneticField.z);
//#endif
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] Magnetic Field:%.2f, %.2f, %.2f.\n", (unsigned int) HAL_GetTick(),
//				value.un.magneticField.x,
//				value.un.magneticField.y,
//				value.un.magneticField.z);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//
//		break;
//
//	case SH2_RAW_ACCELEROMETER:
//#if PRINT_APP_IMU_RAW_ACCL
//		value.un.rawAccelerometer.x,
//		value.un.rawAccelerometer.y,
//		value.un.rawAccelerometer.z;
//#endif
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] RAW Accelerometer:%.2f, %.2f, %.2f.\n", (unsigned int) HAL_GetTick(),
//				value.un.rawAccelerometer.x,
//				value.un.rawAccelerometer.y,
//				value.un.rawAccelerometer.z);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//		break;
//
//	case SH2_ACCELEROMETER:
//		imu->accelerometer.timestamp = t_ms;
//		imu->accelerometer.x = value.un.accelerometer.x;
//		imu->accelerometer.y = value.un.accelerometer.y;
//		imu->accelerometer.z = value.un.accelerometer.z;
//#if PRINT_APP_IMU_ACCL
//		printf("[IMU] Acc: %f %f %f\n",
//				value.un.accelerometer.x,
//				value.un.accelerometer.y,
//				value.un.accelerometer.z);
//#endif
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] Accelerometer:%f, %f, %f.\n", (unsigned int) HAL_GetTick(),
//				value.un.accelerometer.x,
//				value.un.accelerometer.y,
//				value.un.accelerometer.z);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//		break;
//
//	case SH2_ROTATION_VECTOR:
//		imu->rotVectors.timestamp = t_ms;
//		imu->rotVectors.real = value.un.rotationVector.real;
//		imu->rotVectors.i = value.un.rotationVector.i;
//		imu->rotVectors.j = value.un.rotationVector.j;
//		imu->rotVectors.k = value.un.rotationVector.k;
//#if PRINT_APP_IMU_ROT_VEC
//		r = value.un.rotationVector.real;
//		i = value.un.rotationVector.i;
//		j = value.un.rotationVector.j;
//		k = value.un.rotationVector.k;
//		acc_deg = scaleRadToDeg * value.un.rotationVector.accuracy;
//#endif
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] Rotation Vector: r=%f, i=%f, j=%f, k=%f.\n", (unsigned int) HAL_GetTick(),
//				value.un.rotationVector.real,
//				value.un.rotationVector.i,
//				value.un.rotationVector.j,
//				value.un.rotationVector.k);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//		break;
//
//	case SH2_GAME_ROTATION_VECTOR:
//		r = value.un.gameRotationVector.real;
//		i = value.un.gameRotationVector.i;
//		j = value.un.gameRotationVector.j;
//		k = value.un.gameRotationVector.k;
//		break;
//
//	case SH2_GYROSCOPE_CALIBRATED:
//		imu->gyroscope.timestamp = t_ms;
//		imu->gyroscope.x = value.un.gyroscope.x * 180;
//		imu->gyroscope.y = value.un.gyroscope.y * 180;
//		imu->gyroscope.z = value.un.gyroscope.z * 180;
//
//#if PRINT_APP_IMU_CALIBRATED_GYRO
//		x = value.un.gyroscope.x * 180;
//		y = value.un.gyroscope.y * 180;
//		z = value.un.gyroscope.z * 180;
//		printf("[IMU] %8.4f GYRO_CAL: "
//				"x:%0.6f y:%0.6f z:%0.6f\n",
//				t,
//				x, y, z);
//#endif
//		break;
//
//	case SH2_GYROSCOPE_UNCALIBRATED:
//		x = value.un.gyroscopeUncal.x;
//		y = value.un.gyroscopeUncal.y;
//		z = value.un.gyroscopeUncal.z;
//#if PRINT_APP_IMU_UNCALIBRATED_GYRO
//		printf("[IMU] %8.4f GYRO_UNCAL: "
//				"x:%0.6f y:%0.6f z:%0.6f\n",
//				t,
//				x, y, z);
//#endif
//		break;
//
//	case SH2_GYRO_INTEGRATED_RV:
//		// These come at 1kHz, too fast to print all of them.
//		// So only print every 10th one
//		skip++;
//		if (skip == 10) {
//
//			skip = 0;
//#if 0
//			r = value.un.gyroIntegratedRV.real;
//			i = value.un.gyroIntegratedRV.i;
//			j = value.un.gyroIntegratedRV.j;
//			k = value.un.gyroIntegratedRV.k;
//			x = value.un.gyroIntegratedRV.angVelX;
//			y = value.un.gyroIntegratedRV.angVelY;
//			z = value.un.gyroIntegratedRV.angVelZ;
//			printf("[IMU] %8.4f Gyro Integrated RV: "
//					"r:%0.6f i:%0.6f j:%0.6f k:%0.6f x:%0.6f y:%0.6f z:%0.6f\n",
//					t,
//					r, i, j, k,
//					x, y, z);
//#endif
//		}
//		break;
//	default:
//#if PRINTF_APP_IMU_DBG
//		sprintf(string, "%u [APP_IMU] [refreshImuData] Unknown sensor: %d.\n", (unsigned int) HAL_GetTick(), value.sensorId);
//		HAL_UART_Transmit(&huart5, (uint8_t *)string, strlen(string), 25);
//#endif
//		break;
//	}
//}

void IMU_Config_Init()
{
	CONFIG_WAITING_FOR_DECODE(); /* Block on this line untill a configuration is decoded and valid */

	/* Get number of IMU instrument from config */
//	int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_IMU);
	int n = getNumberOfInstrumentSpecificFromConfig(&decodedConfig.conf, SETUP_PRM_COMM_METHOD_BT);

#if PRINTF_APP_IMU_DBG
		sprintf(string, "[APP_IMU] [IMU_Config_Init] Number of instruments with SETUP_PRM_COMM_METHOD_BT: %d.\n", n);
	    xQueueSend(pPrintQueue, string, 0);
#endif

//	if (n==1)
//	{
		/* Retrieved one IMU instrument from the configuration link the the IMU instrument pointer handler to the instrument_config_t struct from
		 * the decodedConfig struct */
		//		n =  getInstrumentFromConfig(&decodedConfig.conf, (instrument_config_t *)&pImuInstrument, SETUP_PRM_COMM_METHOD_IMU);
//		n =  getInstrumentFromConfig(&decodedConfig.conf, &pImuInstrument, SETUP_PRM_COMM_METHOD_IMU);
		n =  getInstrumentFromConfig(&decodedConfig.conf, &pImuInstrument, SETUP_PRM_COMM_METHOD_BT);

//		if (n == 1)
//		{
			/* Correctly retrieved instrument pointer */
//		}
//	}
//	else
//	{
		/* No IMU instrument retrieved or too many in the configuration */
		/* Delete the task of the IMU */
//		osThreadTerminate(imuTaskHandle);
//	}
}

int IMU_Config_Still_Valid(void)
{
	if (decodedConfig.state != CONF_CORRECT)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


#endif
