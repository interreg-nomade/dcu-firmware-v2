/**
 * @file app_sync.h
 * @brief Synchronization between storage and streaming
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 */

#ifndef APP_SYNC_H_
#define APP_SYNC_H_

#include <stdio.h>


void initSyncThread();
void app_sync_notify_from_isr(uint32_t notValue);

#endif /* APP_SYNC_H_ */
