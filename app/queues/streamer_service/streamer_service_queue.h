/**
 * @file streamer_service_queue.h
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef QUEUES_STREAMER_SERVICE_STREAMER_SERVICE_QUEUE_H_
#define QUEUES_STREAMER_SERVICE_STREAMER_SERVICE_QUEUE_H_

typedef enum {
	streamerService_DisableAll = 0,

	streamerService_DisableAndroidStream = 1,
	streamerService_EnableAndroidStream = 2,
	streamingService_ACKAndroidStream = 10,
	streamingService_NACKAndroidStream = 11,

	streamerService_DisableUsbStream = 3,
	streamerService_EnableUsbStream = 4

} streamerServiceAction_t;

typedef struct {

	streamerServiceAction_t action;
	unsigned char nPackets;
	unsigned int  nBytes;

	unsigned int theirCycleCounter;
	unsigned int ourCycleCounter;

	unsigned char destination;
	unsigned char source;

} streamerServiceQueueMsg_t;


int streamer_service_queue_init(void);
int streamer_service_post(streamerServiceQueueMsg_t msg);
int streamer_service_receive(streamerServiceQueueMsg_t * pmsg);

#endif /* QUEUES_CONFIG_SERVICE_CONFIG_SERVICE_QUEUE_H_ */
