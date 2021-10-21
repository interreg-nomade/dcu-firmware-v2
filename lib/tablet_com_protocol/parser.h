/**
 * @file parser.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef CLOUD_PROT_PARSER_H_
#define CLOUD_PROT_PARSER_H_

#include <stdint.h>

#include "defines.h"
#include "frames.h"

typedef enum {
    CPL_NO_MSG,
    CPL_IN_PROGRESS,
    CPL_CORRECT_FRAME,
    CPL_INCORRECT_FRAME,
	CPL_WRONG_CRC
} CPL_OP_RESULT;

typedef enum {
    CPL_IDLE,
    CPL_BUILDING_HEADER,
    CPL_BUILDING_BODY,
    CPL_CHECKING_FRAME

} CPL_PARSER_STATE;

void cpl_init();
//CPL_OP_RESULT cp_prot_decoder(cpl_msg_t * pCplMsg);
CPL_OP_RESULT cpl_prot_decoder(cpl_msg_t * pCplMsg,
		uint32_t actualTime);
void cpl_RxHandler(char c);
unsigned int cpl_RxBufferSize(void);
int cpl_buildFrame(cpl_msg_t * Msg, unsigned int * cx, char * buffer);
int cpl_buildFrameNoPayload(cpl_msg_t * Msg, unsigned int * cx, char * buffer);
void cpl_rst_msg(cpl_msg_t * pCplMsg);


#endif /* CLOUD_PROT_PARSER_H_ le */
