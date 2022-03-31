/*  ____  ____      _    __  __  ____ ___
 * |  _ \|  _ \    / \  |  \/  |/ ___/ _ \
 * | | | | |_) |  / _ \ | |\/| | |  | | | |
 * | |_| |  _ <  / ___ \| |  | | |__| |_| |
 * |____/|_| \_\/_/   \_\_|  |_|\____\___/
 *                           research group
 *                             dramco.be/
 *
 *  KU Leuven - Technology Campus Gent,
 *  Gebroeders De Smetstraat 1,
 *  B-9000 Gent, Belgium
 *
 *         File: battery.h
 *      Created: 11-3-2022
 *       Author: Jona Cappelle
 *      Version: v0.1
 *
 *  Description: Battery voltage measurement
 *
 *  Commissioned by the Interreg NOMADe project
 *
 */

#ifndef __BATTERY_H
#define __BATTERY_H

// #include "stm32h7xx_hal_conf.h"

//#ifdef HAL_ADC_MODULE_ENABLED

void measBatt_Init();
void measBatt(float *value);

static void MX_ADC3_Init(void);


//#endif

#endif
