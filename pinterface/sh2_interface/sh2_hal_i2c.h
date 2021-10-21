/*
 * sh2_hal_i2c.h
 *
 *  Created on: Jun 7, 2019
 *      Author: aclem
 */

#ifndef BNO080_STM32_HILLCREST_SH2_HAL_I2C_H_
#define BNO080_STM32_HILLCREST_SH2_HAL_I2C_H_

#include "i2c.h"

void BNO_I2C_MasterTxCpltCallback(I2C_HandleTypeDef * hi2c);

void BNO_I2C_MasterRxCpltCallback(I2C_HandleTypeDef * hi2c);

void BNO_I2C_ErrorCallback(I2C_HandleTypeDef * hi2c);


#endif /* BNO080_STM32_HILLCREST_SH2_HAL_I2C_H_ */
