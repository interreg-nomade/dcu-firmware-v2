/**
 * @file fc_tx.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef CLOUD_PROT_FC_TX_H_
#define CLOUD_PROT_FC_TX_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* This file contains the source code used for the FC transfer */
/* Trying to use a similiPOO philosophy to make it portable for other interfaces */
typedef enum {
    FC_ERROR = 0,
    FC_INIT_ACK,
    FC_INIT_NACK,
    FC_BURST_ACK,
    FC_BURST_NACK,

} fc_tx_event_t;

typedef enum {
    FC_STATE_ERROR = -1,
    FC_STATE_INIT = 0,
    FC_STATE_WAITING_INIT_PACKET_ANSWER,
    FC_STATE_WAITING_BURST_ANSWER,
    FC_STATE_OVER,
    FC_STATE_TIMEOUT

} fc_tx_state_t;

typedef enum {
    FC_RESSOURCE_FREE,
    FC_RESSOURCE_BUSY

} fc_ressource_state_t;
/* Could be replaced with a mutex in RTOS env. */

struct fc_tx_handler_t {
/* General statistics (constants after init) */
    unsigned int totalBytes;    /* Total number of bytes to send */
    unsigned int totalPackets;  /* Total number of packets to send */
    unsigned int payloadSize;   /* Size of the payload in one packet */
    unsigned int burstSize;     /* Usual size of a burst */
    unsigned int totalBurst;

/* Evolving statistics */
    unsigned int burstSent;      /* Number of burst(s)  sent counter */
    unsigned int packetSent;     /* Number of packet(s) sent counter */
    unsigned int packetsPending; /* Number of packet(s) sent and waiting for an ACK */
    unsigned int offset;         /* In case of NACK; avoid to resend the whole burst
                                    but just the part of the burst starting from the offset */
/* Burst data buffer */
    unsigned char buffer[2048]; /* The buffer that stores a bunch of data divided into
                                 *   packets is stored untill the burst is ACK or a timeout
                                 *   occures (usefull for NACK handling) */
    unsigned int bufferElems;
    unsigned int processedBytes;

    unsigned int startIndex;
    unsigned int endIndex;
    unsigned int bytesToProcess;

/* Callbacks */
    /* This function is called every time an event occurs */
    int (*event)(struct fc_tx_handler_t * self, fc_tx_event_t event, unsigned int val);
    /* This function is called at every begin of a burst transfer */
    int (*refreshData)(struct fc_tx_handler_t * self); /* Actualize the buffer */
    /* This function is called at every begin of a burst transfer */
    int (*txData)(uint8_t * data, unsigned int length); /* Actualize the buffer */
    /* This function is called once the transfer is over */
    int (*txOver)(unsigned short service);

    fc_ressource_state_t ressourceState;
    fc_tx_state_t        currentState;

/* Service ID */
    unsigned short ServiceID;
/* Dest/Source */
    unsigned char destination;
    unsigned char source;
};
typedef struct fc_tx_handler_t fc_tx_handler_t;

void fc_parse_one_packet(unsigned char * source, unsigned int sourcelen, unsigned int packetNumber,
							unsigned char destinationAddress, unsigned char sourceAddress,
							unsigned char * dest, unsigned int * destlen);

void fc_printf_one_packet_in_hex(unsigned char * buffer, unsigned int len);
void fc_send_one_burst(fc_tx_handler_t * pTxHandler);
void fc_tx_calculate_burst_size(struct fc_tx_handler_t * self);
int  exampleEventCallback(struct fc_tx_handler_t * self, fc_tx_event_t event, unsigned int val);


/* Save as a template */
int refreshTxHandlerDataImpl(struct fc_tx_handler_t * self);

/* The function initialize the handler */
void fc_tx_init(fc_tx_handler_t * self);


#ifdef FC_TX_TEST
int main()
{
    //copy_json_file(bigBuffer, &bufferLength);
    /* This prints the buffer that now contains sample1.json */
#if 0
    for (unsigned int i = 0; i<bufferLength; i++)
    {
        printf("%c", bigBuffer[i]);
    }
    printf("\n");
#endif
    unsigned int stopProcess;
    stopProcess = 0;

    fc_tx_handler_t txHandler;
    memset(&txHandler, 0, sizeof(fc_tx_handler_t));

    txHandler.event = exampleEventCallback;

    txHandler.totalBytes = 744;
    txHandler.payloadSize = 248;
    txHandler.burstSize = 5;

    txHandler.refreshData = refreshTxHandlerDataImpl;

    fc_tx_init(&txHandler);

    while (!stopProcess)
    {
        /* The char capture is used to simulate the reception of messages
        (ACK/NACK/INIT_ACK/INIT_NACK) that are handled by the parser */
        char line[128];
        char ch;

        memset(line, 0, 128);
        ch = 0;

        if (fgets(line, sizeof line, stdin) == NULL) {
            printf("Input error.\n");
            //break;
        }
        ch = line[0];
        switch(ch)
        {
            case 'a':
            {
                txHandler.event(&txHandler, FC_BURST_ACK, 0);
                break;
            }
            case 'n':
            {
                unsigned int packet = 0;
                packet = atoi(&line[1]);

                printf("NACK: %d\n", packet);
                txHandler.event(&txHandler, FC_BURST_NACK, packet);

                break;
            }
            case 'i':
            {
                txHandler.event(&txHandler, FC_INIT_ACK, 0);
                break;
            }
            case 'o':
            {
                txHandler.event(&txHandler, FC_INIT_NACK, 0);
                break;
            }
            default:
            {
                /* not handled */
                break;
            }
        }
    }
    return 0;
}
#endif


#endif /* CLOUD_PROT_FC_TX_H_ */
