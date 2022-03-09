/*
 * app_BLEmodule5.h
 *
 *  Created on: 23 feb. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_BLEMODULE5_H_
#define APP_BLEMODULE5_H_

#include "data/structures.h"

/* Call this function to initialize the BLEmodule1_task and start it */
void BLEmodule5_task_init(void);
void sensorHandlerBLEmodule5(imu_100Hz_data_t *SensorEvent);


#endif /* APP_BLEMODULE5_H_ */
