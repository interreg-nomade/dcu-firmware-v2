#ifndef QUEUES_NOTIFICATION_NOTIFICATION_SERVICE_QUEUE_H_
#define QUEUES_NOTIFICATION_NOTIFICATION_SERVICE_QUEUE_H_

#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"
#include <stdint.h>

int notification_service_queue_init(unsigned int nb_elem, size_t size);
int notification_service_post(void * data, size_t size, TickType_t timeToWait);
int notification_service_receive(void * data, size_t size,  TickType_t timeToWait);

#endif /* QUEUES_NOTIFICATION_NOTIFICATION_SERVICE_QUEUE_H_ */
