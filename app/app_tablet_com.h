/**
 * @file app_tablet_com.h
 * @brief Application for the mainboard-tablet communication
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 */

#ifndef APP_TABLET_COM_H_
#define APP_TABLET_COM_H_

#include <stdint.h>

typedef struct {
	unsigned int isOnline;
	unsigned char tabletAddress;
	unsigned char mainboardAddress;
	unsigned int streamingEnabled;
} upLinkStatus_t;

extern upLinkStatus_t androidTabLinkHandler;

void cpl_init_rx_task(void);
void tablet_com_send_shutdown_message();
void tablet_com_temporary_raise_flag(void);
void tablet_com_start_measurement_callback(uint64_t startTime);
void tablet_com_stop_measurement_callback(uint64_t stopTime);
void app_tablet_com_prepare_for_shutdown(void);
void app_ftdi_error_increment(void);
int tablet_com_is_online(void);
int tablet_com_set_state(unsigned int state);

void app_tablet_com_notify(uint32_t notValue);
void app_tablet_com_notify_from_isr(uint32_t notValue);


#endif /* APP_TABLET_COM_H_ */
