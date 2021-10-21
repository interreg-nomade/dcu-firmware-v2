/*
 * android_com_queue.h
 *
 *  Created on: Apr 7, 2020
 *      Author: ali
 */

#ifndef QUEUES_ANDROID_DEVICE_COM_ANDROID_COM_QUEUE_H_
#define QUEUES_ANDROID_DEVICE_COM_ANDROID_COM_QUEUE_H_

int android_com_queue_init(void);
int android_com_receive(measurementsServiceQueueMsg_t * pmsg);
int android_com_post(measurementsServiceQueueMsg_t msg);


#endif /* QUEUES_ANDROID_DEVICE_COM_ANDROID_COM_QUEUE_H_ */
