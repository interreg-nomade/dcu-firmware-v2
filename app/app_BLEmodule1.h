/*
 * app_BLEmodule1.h
 *
 *  Created on: 21 feb. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_BLEMODULE1_H_
#define APP_BLEMODULE1_H_

#include "data/structures.h"

/* Call this function to initialize the BLEmodule1_task and start it */
void BLEmodule1_task_init(void);
void sensorHandlerBLEmodule1(imu_100Hz_data_t *SensorEvent);

#endif /* APP_BLEMODULE1_H_ */
