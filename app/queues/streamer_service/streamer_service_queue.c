/**
 * @file streamer_service_queue.c
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019, Revised in August 2019
 */
#include <queues/streamer_service/streamer_service_queue.h>
#include <stdint.h>
/* FreeRTOS includes */
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"


#define STREAMER_SERVICE_QUEUE_SIZE 20

static osMessageQId streamerServiceQueue;

/**
 * @brief The function creates the queue
 * @return 1 in case of success, 0 otherwise
 */
int streamer_service_queue_init(void)
{
  streamerServiceQueue = NULL; /* Make sure the confServiceQueue handler is NULL */
  /* The queue can contain up to the value defined by STREAMER_SERVICE_QUEUE_SIZE pointers of streamerServiceQueueMsg_t struct */
  streamerServiceQueue = xQueueCreate(STREAMER_SERVICE_QUEUE_SIZE, sizeof(streamerServiceQueueMsg_t)); /* Create the queue */
  if (streamerServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  else
  { /* Queue created correctly */
	return 1;
  }
}

/**
 * @brief Post a message on the streamer queue
 * @param msg message to post
 * @return 1 in case of success, 0 otherwise
 */
int streamer_service_post(streamerServiceQueueMsg_t msg)
{
  streamerServiceQueueMsg_t sMsg;
  memset(&sMsg, 0, sizeof(streamerServiceQueueMsg_t));
  if (streamerServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  sMsg = msg; /* Copy the message */
  if(xQueueSend(streamerServiceQueue, (void *) &sMsg, (TickType_t) 10) != pdPASS)
  { /* Failed to post the message, even after 10 ticks. */
	return 0;
  }
  return 1;
}


/**
 * @brief Try to retrieve a message from the streamerService queue
 * @return a streamerServiceQueue element. It is set to something >0 in case of success (use a if)
 */
int streamer_service_receive(streamerServiceQueueMsg_t * pmsg)
{
  streamerServiceQueueMsg_t sMsg ;
  memset(&sMsg, 0, sizeof(streamerServiceQueueMsg_t));
  if (streamerServiceQueue == NULL)
  { /* Failure */
	return 0;
  }
  if(xQueueReceive(streamerServiceQueue, (void*) &sMsg, (TickType_t) 0 ))
  { /* Correctly received a message, sMsg now holds the received message */
	*pmsg = sMsg;
	return 1;
  }
  return 0;
}

