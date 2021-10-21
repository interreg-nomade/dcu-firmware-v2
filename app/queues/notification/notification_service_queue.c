#include "notification_service_queue.h"

static QueueHandle_t notificationQueue;

int notification_service_queue_init(unsigned int nb_elem, size_t size)
{
	notificationQueue = NULL;
	notificationQueue = xQueueCreate( nb_elem, size );

	if(notificationQueue == NULL)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}


int notification_service_post(void * data, size_t size, TickType_t timeToWait)
{
	if(notificationQueue == NULL)
	{
		return 0;
	}
	else
	{
		char * temp = malloc(size);
		memset((void *)temp, 0, size);

		memcpy((void *)temp, data, size);

		if( xQueueSend( notificationQueue, temp, timeToWait ) != pdPASS)
		{
			free(temp);
			return 0;
		}

		free(temp);

		return 1;
	}

}


int notification_service_receive(void * data, size_t size,  TickType_t timeToWait)
{
	if(notificationQueue == NULL)
	{
		return 0;
	}

	if( xQueueReceive( notificationQueue, data, timeToWait))
	{
		return 1;
	}

	return 0;

}
