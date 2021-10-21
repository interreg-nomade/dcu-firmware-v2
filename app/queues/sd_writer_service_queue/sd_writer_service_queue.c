/**
 * @file streamer_service_queue.c
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
#include "sd_writer_service_queue.h"
#include <stdint.h>
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"


#define SD_WRITER_SERVICE_QUEUE_SIZE 20

static osMessageQId sdWriterServiceQueue;

/**
 * @brief The function creates the queue
 * @return 1 in case of success, 0 otherwise
 */
int sdwriter_service_queue_init(void)
{
  sdWriterServiceQueue = NULL; /* Make sure the sdWriterServiceQueue handler is NULL */
  /* The queue can contain up to the value defined by SD_WRITER_SERVICE_QUEUE_SIZE pointers of sdWriterServiceQueueMsg_t struct */
  sdWriterServiceQueue = xQueueCreate(SD_WRITER_SERVICE_QUEUE_SIZE, sizeof(sdWriterServiceQueueMsg_t)); /* Create the queue */
  if (sdWriterServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  else
  { /* Queue created correctly */
	return 1;
  }
}

/**
 * @brief Post a message on the sd writer queue
 * @param msg message to post
 * @return 1 in case of success, 0 otherwise
 */
int sdwriter_service_post(sdWriterServiceQueueMsg_t msg)
{
  sdWriterServiceQueueMsg_t sMsg;
  memset(&sMsg, 0, sizeof(sdWriterServiceQueueMsg_t));
  if (sdWriterServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  sMsg = msg; /* Copy the message */
  if( xQueueSend(sdWriterServiceQueue, (void *) &sMsg, (TickType_t) 0) != pdPASS)
  { /* Failed to post the message, even after 10 ticks. */
	return 0;
  }
  return 1;
}

/**
 * @brief Try to retrieve a message from the sdWriterServiceQueueMsg_t queue
 * @return a sdWriterServiceQueueMsg_t element. It is set to something >0 in case of success (use a if)
 */
int sdwriter_service_receive(sdWriterServiceQueueMsg_t * pmsg)
{
  sdWriterServiceQueueMsg_t sMsg ;
  memset(&sMsg, 0, sizeof(sdWriterServiceQueueMsg_t));
  if (sdWriterServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  if( xQueueReceive( sdWriterServiceQueue, (void*) &sMsg, portMAX_DELAY ))
  { /* Correctly received a message, sMsg now holds the received message */
	*pmsg = sMsg;
	return 1;
  }
  return 0;
}

