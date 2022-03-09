/*
 * app_BLEmodule3.h
 *
 *  Created on: 23 feb. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_BLEMODULE3_H_
#define APP_BLEMODULE3_H_

#include "data/structures.h"

/* Call this function to initialize the BLEmodule1_task and start it */
void BLEmodule3_task_init(void);
void sensorHandlerBLEmodule3(imu_100Hz_data_t *SensorEvent);


#endif /* APP_BLEMODULE3_H_ */
