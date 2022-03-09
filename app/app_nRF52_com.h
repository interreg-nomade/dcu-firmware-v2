/*
 * app_nRF52_com.h
 *
 *  Created on: 25 Nov 2021
 *      Author: sarah
 */

#ifndef APP_NRF52_COM_H_
#define APP_NRF52_COM_H_

#define MAX_nRF52_PAYLOAD_LENGTH 128
#define NRF52_START_DELIMITER   0x73

typedef struct {
    unsigned char commandType;
	unsigned char command;
    unsigned int length;
    unsigned char DU[MAX_nRF52_PAYLOAD_LENGTH];
    unsigned char CS;
} nRF52_msg_t;

typedef enum {
	nRF52_NO_MSG,
	nRF52_IN_PROGRESS,
	nRF52_CORRECT_FRAME,
	nRF52_INCORRECT_FRAME,
	nRF52_WRONG_CS
} nRF52_DECODER_RESULT;

typedef enum {
	nRF52_IDLE,
	nRF52_BUILDING_HEADER,
	nRF52_BUILDING_BODY,
	nRF52_CHECKING_FRAME
} nRF52_PARSER_STATE;

nRF52_DECODER_RESULT nRF52_prot_decoder(nRF52_msg_t * BTMsg);
int nRF52_calculate_cs(nRF52_msg_t * BTMsg);
void nRF52_rst_msg(nRF52_msg_t * BTMsg);
void nRF52_RxHandler(char c);
void nRF52_init(void);
int nRF52_buildFrame(nRF52_msg_t * nRF52Msg, unsigned int * cx, char * buffer);
void nRF52_init_rx_task(void);
void nRF52_RxHandler(char c);
void nRF52_rst_msg(nRF52_msg_t * BTMsg);

#endif /* APP_NRF52_COM_H_ */
