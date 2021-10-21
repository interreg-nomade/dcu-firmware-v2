/**
 * @file app_storage.h
 * @brief Streaming service queue
 * @author  Yncrea HdF - ISEN Lille / Alexis.C, Ali O.
 * @version 0.1
 * @date September, October 2019
 */
#ifndef STORAGE_RTOS_STORAGE_H_
#define STORAGE_RTOS_STORAGE_H_

#include <stdint.h>

#define APP_STORAGE_NOTIF_STOP_STORING 0x01
#define APP_STORAGE_NOTIF_CYCLE_COUNTER 0x02
#define APP_STORAGE_NOTIF_NEW_MEAS_LIST_FILE 0x04

/* Public functions */
int rtos_measurement_storage_init();
void app_storage_notify(uint32_t notValue);
void app_storage_notify_from_isr(uint32_t notValue);
int app_storage_active();
void app_storage_prepare_for_shutdown(void);

#endif /* STORAGE_RTOS_STORAGE_H_ */
