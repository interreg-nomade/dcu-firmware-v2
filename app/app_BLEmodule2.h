/*
 * app_BLEmodule2.h
 *
 *  Created on: 22 feb. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_BLEMODULE2_H_
#define APP_BLEMODULE2_H_

#include "data/structures.h"

/* Call this function to initialize the BLEmodule1_task and start it */
void BLEmodule2_task_init(void);
void sensorHandlerBLEmodule2(imu_100Hz_data_t *SensorEvent);


#endif /* APP_BLEMODULE2_H_ */
