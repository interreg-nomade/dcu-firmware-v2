/**
 * @file config_service_queue.c
 * @brief Configuration service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */

#include "config_service_queue.h"

/* Standard lib */
#include <stdint.h>
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"


#define CONF_SERVICE_QUEUE_SIZE 4

static osMessageQId confServiceQueue;

/**
 * @brief The function creates the queue
 * @return 1 in case of success, 0 otherwise
 */
int conf_service_queue_init(void)
{
	/* Make sure the confServiceQueue handler is NULL */
	confServiceQueue = NULL;

	/* Create the queue */
	/* The queue can contain up to the value defined by MAIN_DATA_QUEUE_SIZE pointers of mainDataQueueMsg_t struct */
	confServiceQueue = xQueueCreate( CONF_SERVICE_QUEUE_SIZE, sizeof(configServiceQueueMsg_t ) );

	if (confServiceQueue == NULL)
	{
		/* Failure */
		return 0;
	}
	else
	{
		/* Queue created correctly */
		return 1;
	}
}

/**
 * @brief Post a message on the configuration service queue
 * @param msg message to post
 * @return 1 in case of success, 0 otherwise
 */
int conf_service_post(configServiceQueueMsg_t msg)
{
	configServiceQueueMsg_t sMsg = 0;

	if (confServiceQueue == NULL)
	{
		/* Failure */
		return 0;
	}

	sMsg = msg; /* Copy the message */

	if( xQueueSend( confServiceQueue, ( void * ) &sMsg, ( TickType_t ) 10 ) != pdPASS )
	{
	    /* Failed to post the message, even after 10 ticks. */
		return 0;
	}

	return 1;
}


/**
 * @brief Try to retrieve a message from the configService queue
 * @return a configServiceQueueMsg_t element. It is set to something >0 in case of success (use a if)
 */
configServiceQueueMsg_t conf_service_receive()
{
	configServiceQueueMsg_t sMsg = 0;

	if (confServiceQueue == NULL)
	{
		/* Failure */
		return 0;
	}


	if( xQueueReceive( confServiceQueue, (void*) &sMsg, ( TickType_t ) 10 ))
	{
	    /* Correctly received a message, sMsg now holds the received message */
		return sMsg;
	}

	return 0;
}

