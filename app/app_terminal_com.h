/*
 * app_terminal_com.h
 *
 *  Created on: 12 Sep 2020
 *      Author: sarah
 */

#ifndef APP_TERMINAL_COM_H_
#define APP_TERMINAL_COM_H_

#define MAX_COM_PAYLOAD_LENGTH 128

typedef enum {
    COM_NO_DATA,
    COM_CORRECT_DATA,
    COM_INCORRECT_DATA,
} COM_RESULT;

typedef struct {
    unsigned char lenght;
    unsigned char DU[MAX_COM_PAYLOAD_LENGTH];
} com_msg_t;

void com_init_rx_task(void);
void com_RxHandler(char c);
void terminal_com_init();
void com_rst_msg(com_msg_t * pComMsg);


#endif /* APP_TERMINAL_COM_H_ */
