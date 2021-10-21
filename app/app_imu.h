/**
 * @file app_imu.h
 * @brief GPS Application file
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date April 2019
 * Refactor in August 2019
 *
 * Contains the GPS application thread - get data, decode, and push it into the real-time data structure
 */
#ifndef APP_IMU_H_
#define APP_IMU_H_


#include <data/structures.h>

/* Call this function to init the IMU Task and start it */
void imu_task_init(void);
void sensorHandler(imu_100Hz_data_t *SensorEvent);


#endif /* APP_IMU_H_ */
