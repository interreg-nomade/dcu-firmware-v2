/**
 * @file config_service_queue.h
 * @brief Configuration service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */


#ifndef QUEUES_CONFIG_SERVICE_CONFIG_SERVICE_QUEUE_H_
#define QUEUES_CONFIG_SERVICE_CONFIG_SERVICE_QUEUE_H_

typedef enum {
	confService_NONE = 0,
	confService_RAW_RDY_FOR_DECODE_BOOTUP = 1,
	confService_RAW_RDY_FOR_DECODE = 2,

	confService_STREAM_JSON = 3

} configServiceQueueMsg_t;

int conf_service_queue_init(void);
int conf_service_post(configServiceQueueMsg_t msg);
configServiceQueueMsg_t conf_service_receive();

#endif /* QUEUES_CONFIG_SERVICE_CONFIG_SERVICE_QUEUE_H_ */
