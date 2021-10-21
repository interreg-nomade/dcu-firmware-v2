/**
 * @file app_init.h
 * @brief GPS Application file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date April 2019
 * Refactor in August 2019
 *
 * Contains the initialisation part of the project - pre-RTOS running inits, init & monitoring task
 */

#ifndef APP_INIT_H_
#define APP_INIT_H_

#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "semphr.h"

#include "data/structures.h"
#include "queues/config_service/config_service_queue.h"

/* Initialisation function (start the threads) */
void initMainTask();

/* Threads */
void dataMaintainerThread  (const void * params);

/* Initialisation of the whole project */
void project_init(void);

/* This structure is maintained by the "dataMaintainer" thread that retrieves messages
 * from the main data pipe.
 * Every access should use the mutex to provide exclusive access and avoid any corruption
 */
typedef struct {
	data_container_t  data;
	SemaphoreHandle_t mutex;

	unsigned int (*get)    (void);
	void		 (*release)(void);

} protectedDataContainer_t;

extern protectedDataContainer_t protectedData;
extern volatile time_t epoch, epochBootTime;
extern unsigned int bootTime;

#endif /* APP_INIT_H_ */
