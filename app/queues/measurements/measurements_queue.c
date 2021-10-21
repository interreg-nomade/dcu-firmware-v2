/**
 * @file measurement_queue.c
 * @brief Measurement service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date October 2019
 */
#include <string.h>
#include <stdint.h>
#include "measurements_queue.h"

/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"


#define MEASUREMENTS_QUEUE_SIZE 10

static osMessageQId measurementsServiceQueue;

/**
 * @brief The function creates the queue
 * @return 1 in case of success, 0 otherwise
 */
int measurements_service_queue_init(void)
{
	/* Make sure the confServiceQueue handler is NULL */
	measurementsServiceQueue = NULL;

	/* Create the queue */
	/* The queue can contains up to the value defined by MAIN_DATA_QUEUE_SIZE pointers of mainDataQueueMsg_t struct */
	measurementsServiceQueue = xQueueCreate( MEASUREMENTS_QUEUE_SIZE, sizeof(measurementsServiceQueueMsg_t) );

	if (measurementsServiceQueue == NULL)
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
 * @brief Post a message on the measurements queue
 * @param msg message to post
 * @return 1 in case of success, 0 otherwise
 */
int measurements_service_post(measurementsServiceQueueMsg_t msg)
{
	measurementsServiceQueueMsg_t sMsg;
	memset(&sMsg, 0, sizeof(measurementsServiceQueueMsg_t));

	if (measurementsServiceQueue == NULL)
	{
		/* Failure */
		return 0;
	}

	sMsg = msg; /* Copy the message */

	if( xQueueSend( measurementsServiceQueue, ( void * ) &sMsg, ( TickType_t ) 0 ) != pdPASS )
	{
	    /* Failed to post the message, even after 10 ticks. */
		//printf("Failed to post message in %s\n", __func__);
		return 0;
	}

	return 1;
}


/**
 * @brief Try to retrieve a message from the streamerService queue
 * @return a measurementsServiceQueueMsg_t element. It is set to something >0 in case of success (use a if)
 */
int measurements_service_receive(measurementsServiceQueueMsg_t * pmsg)
{
	measurementsServiceQueueMsg_t sMsg ;
	memset(&sMsg, 0, sizeof(measurementsServiceQueueMsg_t));

	if (measurementsServiceQueue == NULL)
	{
		/* Failure */
		return 0;
	}


	if( xQueueReceive( measurementsServiceQueue, (void*) &sMsg, ( TickType_t ) 0))
	{
	    /* Correctly received a message, sMsg now holds the received message */
		*pmsg = sMsg;
		return 1;
	}

	return 0;
}

