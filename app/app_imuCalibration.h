/*
 * app_imuCalibration.h
 *
 *  Created on: 29 apr. 2022
 *      Author: Sarah Goossens
 */

#ifndef APP_IMUCALIBRATION_H_
#define APP_IMUCALIBRATION_H_

void IMUCalibrationTask_init(void);  /* Call this function to initialize the IMU calibration task and start it */
void IMUCalibrationTask(const void * params);

#endif /* APP_IMUCALIBRATION_H_ */
