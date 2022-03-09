/*
 * app_BLEmodule6.h
 *
 *  Created on: 23 feb. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_BLEMODULE6_H_
#define APP_BLEMODULE6_H_

#include "data/structures.h"

/* Call this function to initialize the BLEmodule1_task and start it */
void BLEmodule6_task_init(void);
void sensorHandlerBLEmodule6(imu_100Hz_data_t *SensorEvent);


#endif /* APP_BLEMODULE6_H_ */
