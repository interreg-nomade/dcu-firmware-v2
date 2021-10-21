/**
 * @file frames.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef TABLET_COM_PROTOCOL_FRAMES_H_
#define TABLET_COM_PROTOCOL_FRAMES_H_

#include "defines.h"

typedef struct {
    unsigned char lenght;

    unsigned char destinationAddress;
    unsigned char sourceAddress;

    unsigned char destinationService;
    unsigned char sourceService;

    unsigned char FC;

    unsigned char DU[MAX_CPL_PAYLOAD_LENGTH]; // The first 3 byte corresponding to the number of packets and number of bytes.

    unsigned char FCS;

    unsigned int payloadLength;

    unsigned short SERVICE;
    unsigned short DASA;

    unsigned char flowCode; /* Flow code is the byte sent after an ACK (0x80) or a NACK (0xFE) at the DSAP position */

    unsigned short packetNumber;

} cpl_msg_t;

void cpl_util_set_service(cpl_msg_t * Msg, short service);

#endif /* TABLET_COM_PROTOCOL_FRAMES_H_ */
