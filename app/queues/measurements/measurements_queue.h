/**
 * @file streamer_service_queue.h
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef QUEUES_MEASUREMENTS_SERVICE_MEASUREMENT_SERVICE_QUEUE_H_
#define QUEUES_MEASUREMENTS_SERVICE_MEASUREMENT_SERVICE_QUEUE_H_

#include <stdint.h>

typedef enum {
	mesurement_NoAction						= 0,
	measurement_SetMeasurementID			= 1,
	measurement_SetMeasurementList			= 2,
	measurement_StartMeasurement			= 3,
	measurement_StopMeasurement				= 4,
	measurement_Prepare4Shutdown			= 5
} measurementServiceAction_t;

typedef enum{
	measurement_StatusOriginIDLE 			= 0,
	measurement_StatusOriginAndroidDevice 	= 1, /* if SERVICE_REQUEST_START_MEASUREMENT is being requested by Android device,
													then serviceQueueMsg->status = measurement_StatusOriginAndroidDevice
	 	 	 	 	 	 	 	 	 	 	 	 	 	 serviceQueueMsg->action = measurement_StartMeasurement */
	measurement_StatusOriginMeasurementList = 2,
}measurementServiceStatus_t;

typedef struct {
	measurementServiceStatus_t status;
	measurementServiceAction_t action;
	unsigned int measurementId;
	uint64_t startTime;
	uint64_t stopTime;
} measurementsServiceQueueMsg_t;


int measurements_service_queue_init(void);
int measurements_service_post(measurementsServiceQueueMsg_t msg);
int measurements_service_receive(measurementsServiceQueueMsg_t * pmsg);

#endif /* QUEUES_MEASUREMENTS_SERVICE_MEASUREMENT_SERVICE_QUEUE_H_ */
