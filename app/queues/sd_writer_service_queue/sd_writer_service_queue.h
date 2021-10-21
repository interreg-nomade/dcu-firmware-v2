/**
 * @file streamer_service_queue.h
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#ifndef QUEUES_SD_SERVICE_QUEUE_H_
#define QUEUES_SD_SERVICE_QUEUE_H_

#include <stdint.h>

typedef enum {
	sdWriterOp1,
} sdWriterServiceAction_t;

typedef struct {
	sdWriterServiceAction_t action;
	uint8_t * buffer;
	uint8_t * state;
	uint32_t * elems;
} sdWriterServiceQueueMsg_t;


int sdwriter_service_queue_init(void);
int sdwriter_service_post(sdWriterServiceQueueMsg_t msg);
int sdwriter_service_receive(sdWriterServiceQueueMsg_t * pmsg);

#endif /* QUEUES_SD_SERVICE_QUEUE_H_ */
