/**
 * @file app_streamer.h
 * @brief Streaming to tablet application
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 */
#ifndef APP_STREAMER_H_
#define APP_STREAMER_H_

#define APP_STREAMER_NOTIF_CYCLE_COUNTER 0x02

#include <stdint.h>

/* Initialisation function (start the threads) */
void tabletDataStreamThread(const void *params);
void initStreamerThread(void);
int app_streamer_usb_stream_enabled(void);
void app_streamer_notify(uint32_t notValue);
void app_streamer_notify_from_isr(uint32_t notValue);

#endif /* APP_STREAMER_H_ */
