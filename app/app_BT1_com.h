/*
 * app_BT1_com.h
 *
 *  Created on: 19 Mar 2021
 *      Author: sarah
 */

#ifndef APP_BT1_COM_H_
#define APP_BT1_COM_H_

#define MAX_BT_PAYLOAD_LENGTH 964 // (in high throughput mode, otherwise 243)
#define BT_START_DELIMITER 0x02

typedef struct {
    unsigned char command;
    unsigned int length;
    unsigned char DU[MAX_BT_PAYLOAD_LENGTH];
    unsigned char CS;
} BT_msg_t;

typedef enum {
    BT_NO_MSG,
    BT_IN_PROGRESS,
    BT_CORRECT_FRAME,
    BT_INCORRECT_FRAME,
	BT_WRONG_CS
} BT_DECODER_RESULT;

typedef enum {
    BT_IDLE,
	BT_BUILDING_HEADER,
    BT_BUILDING_BODY,
    BT_CHECKING_FRAME
} BT_PARSER_STATE;

BT_DECODER_RESULT bt_prot_decoder(BT_msg_t * BTMsg);
int bt_calculate_cs(BT_msg_t * BTMsg);
void bt_rst_msg(BT_msg_t * BTMsg);
void bt_RxHandler(char c);
void bt_init(void);
int bt_buildFrame(BT_msg_t * BTMsg, unsigned int * cx, char * buffer);
void bt_init_rx_task(void);
void bt_RxHandler(char c);
void bt_rst_msg(BT_msg_t * BTMsg);

#endif /* APP_BT1_COM_H_ */
