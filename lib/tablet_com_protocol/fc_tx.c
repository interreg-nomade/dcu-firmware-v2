/*
 * fc_tx.c
 *
 *  Created on: Dec 21, 2018
 *      Author: aclem
 *
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 *
 */


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "fc_tx.h"
#include "fc_frames.h"
#include "defines.h"

#include "usart.h" // to declare UART transmit
#include "app_init.h" // to declare QueueHandle_t


#define DBG_FC_TX_INFOS_ENABLED 1

extern char string[];
extern QueueHandle_t pPrintQueue;

/* This file contains the source code used for the FC transfer */
/* Trying to use a similiPOO philosophy to make it portable for other interfaces */

void fc_parse_one_packet(unsigned char * source, unsigned int sourcelen, unsigned int packetNumber,
										unsigned char destinationAddress, unsigned char sourceAddress,
        								unsigned char * dest, unsigned int * destlen)
{
    dest[0] = 0x68;
    dest[1] = sourcelen + 7;
    dest[2] = sourcelen + 7;
    dest[3] = 0x68;
    dest[4] = destinationAddress;
    dest[5] = sourceAddress;
    dest[6] = FLOW_CONTROL_FUNCTION_CODE_DATA; /* FC */
    dest[7] = packetNumber >> 24 & 0xff;
    dest[8] = packetNumber >> 16 & 0xff;
    dest[9] = packetNumber >>  8 & 0xff;
    dest[10] = packetNumber      & 0xff;
    for (unsigned int i = 0; i<sourcelen; i++)
    {
        dest[i+11] = source[i];
    }
    unsigned char checksum = 0;
    for (unsigned int i = 4; i < (sourcelen+7); i++)
    {
        checksum += dest[i];
    }
    dest[10+sourcelen] = checksum;
    dest[11+sourcelen] = 0x16;

    *destlen = 12+sourcelen;
}

void fc_printf_one_packet_in_hex(unsigned char * buffer, unsigned int len)
{
//#if DBG_FC_TX_INFOS_ENABLED
	sprintf(string, "[FC] [fc_printf_one_packet_in_hex] Packet printer: ");
    sprintf(string, "");
      char DUString[3];
      for (unsigned int i = 0; i<len; i++)
      {
    	  if (strlen(string) < 147)
    	  {
    	    	sprintf(DUString, "%c", buffer[i]);
	    	  strcat(string, DUString);
    	  }
      }
	      strcat(string,"\n");
      xQueueSend(pPrintQueue, string, 0);
//#endif
}

void fc_send_one_burst(fc_tx_handler_t * pTxHandler)
{
    unsigned int nPackets         = (pTxHandler->packetSent+pTxHandler->packetsPending) - pTxHandler->packetSent;
    unsigned int nBytesToProcess  = pTxHandler->bufferElems;
    unsigned int nBytesProcessed  = 0;
    unsigned int packetNumber     = pTxHandler->packetSent;
    unsigned int packetStartIndex = 0;
    unsigned int packetEndIndex   = 0;
    unsigned int effectiveOffset  = (pTxHandler->offset);
    unsigned int startPacket      = 0;

    startPacket = pTxHandler->packetsPending - effectiveOffset;

#if DBG_FC_TX_INFOS_ENABLED
	sprintf(string, "[FC] [fc_send_one_burst] Packets pending %d\n", pTxHandler->packetsPending);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[FC] [fc_send_one_burst] One burst: send %d to %d\n", pTxHandler->packetSent, pTxHandler->packetSent+pTxHandler->packetsPending);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[FC] [fc_send_one_burst] Has to send %d bytes in this burst\n", nBytesToProcess);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[FC] [fc_send_one_burst] Packet pointer starts at: %d\n", startPacket);
	xQueueSend(pPrintQueue, string, 0);
	sprintf(string, "[FC] [fc_send_one_burst] Sending %d packets\n", nPackets);
	xQueueSend(pPrintQueue, string, 0);
#endif


    for (unsigned int i=0; i<nPackets; i++)
    {
        unsigned int bytesForThisPacket = 0;
        ++packetNumber;

        if ((nPackets != pTxHandler->burstSize) && ((i+1) == nPackets))
        {
            bytesForThisPacket = nBytesToProcess - nBytesProcessed;
            nBytesProcessed += bytesForThisPacket;

        }
        else
        {
            bytesForThisPacket = pTxHandler->payloadSize;
            nBytesProcessed   += bytesForThisPacket;
        }

        packetStartIndex = nBytesProcessed-bytesForThisPacket;
        packetEndIndex   = nBytesProcessed-1;
        /* TODO: optimize this */
        unsigned char dest[512];
        unsigned int  destLen = 0;
        memset(dest, 0, 512);

        if ( i >= startPacket)
        {
            fc_parse_one_packet(pTxHandler->buffer+packetStartIndex,
                                    bytesForThisPacket,
                                    packetNumber,
									pTxHandler->destination,
									pTxHandler->source,
                                    dest,
                                    &destLen);
            pTxHandler->txData(dest, destLen);
#if DBG_FC_TX_INFOS_ENABLED
        	sprintf(string, "[FC] [fc_send_one_burst] Packet #%d contains %d bytes from %d to %d\n", packetNumber,  bytesForThisPacket, packetStartIndex, packetEndIndex);
        	xQueueSend(pPrintQueue, string, 0);
        	sprintf(string, "[FC] [fc_send_one_burst] nBytesProcessed:%d\n", nBytesProcessed);
        	xQueueSend(pPrintQueue, string, 0);
#endif

        }

#if DBG_FC_TX_INFOS_ENABLED
        fc_printf_one_packet_in_hex(dest, destLen);
#endif
    }
}

void fc_tx_calculate_burst_size(struct fc_tx_handler_t * self)
{
    unsigned int start = 0;
    unsigned int end   = 0;

    if ((self->totalPackets - self->packetSent) < self->burstSize)
    {
        start = self->burstSent     *  (self->burstSize * self->payloadSize);
        end   = self->totalBytes -1;
#if DBG_FC_TX_INFOS_ENABLED
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point :: start:%d  end:%d \n", start, end);
    	xQueueSend(pPrintQueue, string, 0);
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
    	xQueueSend(pPrintQueue, string, 0);
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] And copy %d bytes\n", self->burstSize * self->payloadSize);
    	xQueueSend(pPrintQueue, string, 0);
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] Last burst to send\n");
    	xQueueSend(pPrintQueue, string, 0);
#endif
        self->packetsPending = (self->burstSize % (self->totalPackets-self->packetSent) +1);
    }
    else if ((self->totalPackets - self->packetSent) == self->burstSize)
    {
        start = self->burstSent     *  (self->burstSize * self->payloadSize);
        end   = self->totalBytes -1;
#if DBG_FC_TX_INFOS_ENABLED
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point :: start:%d  end:%d \n", start, end);
    	xQueueSend(pPrintQueue, string, 0);
    	sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
    	xQueueSend(pPrintQueue, string, 0);
        sprintf(string, "[FC] [fc_tx_calculate_burst_size] And copy %d bytes\n", self->burstSize * self->payloadSize);
    	xQueueSend(pPrintQueue, string, 0);
        sprintf(string, "[FC] [fc_tx_calculate_burst_size] Packets to send exactly equal to size of one burst\n");
    	xQueueSend(pPrintQueue, string, 0);
#endif
        self->packetsPending = self->burstSize;

    }
    else
    {
        /* Read from N to N+(burstSize*payloadLength) */
        /* N depends of the burst(s) sent */
        start = self->burstSent     *  (self->burstSize * self->payloadSize);
        end   = (self->burstSent+1) *  (self->burstSize * self->payloadSize) -1;

        self->packetsPending = (self->burstSize % (self->totalPackets-self->packetSent));
#if DBG_FC_TX_INFOS_ENABLED
        sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point :: start:%d  end:%d \n", start, end);
    	xQueueSend(pPrintQueue, string, 0);
        sprintf(string, "[FC] [fc_tx_calculate_burst_size] Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
    	xQueueSend(pPrintQueue, string, 0);
        sprintf(string, "[FC] [fc_tx_calculate_burst_size] And copy %d bytes\n", self->burstSize * self->payloadSize);
    	xQueueSend(pPrintQueue, string, 0);
#endif
    }
    self->startIndex = start;
    self->endIndex   = end;
    self->bytesToProcess = end-start+1; /* fix the offset */
    //self->offset = self->burstSize;
    self->offset = self->packetsPending;
}

int exampleEventCallback(struct fc_tx_handler_t * self, fc_tx_event_t event, unsigned int val)
{

	switch (self->currentState)
    {
        case FC_STATE_WAITING_INIT_PACKET_ANSWER:
        {
            if (event == FC_INIT_ACK)
            {
#if DBG_FC_TX_INFOS_ENABLED
            	sprintf(string, "[FC] [exampleEventCallback] Init packet have been acknowledged, we can switch to burst tx state\n");
            	xQueueSend(pPrintQueue, string, 0);
#endif
                fc_tx_calculate_burst_size(self);
                self->refreshData(self);
                fc_send_one_burst(self);
                self->currentState = FC_STATE_WAITING_BURST_ANSWER;
            }

            else if  (event == FC_INIT_NACK)
            {
                /* Error */
                break;
            }
            else
            {
#if DBG_FC_TX_INFOS_ENABLED
                sprintf(string, "[FC] [exampleEventCallback] Message not handled in that state\n");
            	xQueueSend(pPrintQueue, string, 0);
#endif
                break;
            }
            break;
        }

        case FC_STATE_WAITING_BURST_ANSWER:
        {
            if (event == FC_BURST_ACK)
            {
            	/* val contains the ack number */
            	/* if val == awaited ack number */
            	if (val == (self->packetSent + self->packetsPending))
            	{
					self->burstSent++;
					self->packetSent+= self->packetsPending;

#if DBG_FC_TX_INFOS_ENABLED
	                sprintf(string, "[FC] [exampleEventCallback] Burst have been ACK\n");
	            	xQueueSend(pPrintQueue, string, 0);
	                sprintf(string, "[FC] [exampleEventCallback] Status: packets sent: %d/%d - burst sent: %d/%d\n", self->packetSent, self->totalPackets,
																					self->burstSent, self->totalBurst);
	            	xQueueSend(pPrintQueue, string, 0);
#endif

					/* This might be the last burst */
					if (self->packetSent == self->totalPackets)
					{
						self->currentState = FC_STATE_OVER;
#if DBG_FC_TX_INFOS_ENABLED
		                sprintf(string, "[FC] [exampleEventCallback] Transfer is over %d\n", self->totalPackets);
		            	xQueueSend(pPrintQueue, string, 0);
#endif

					}
					else if (self->packetSent < self->totalPackets)
					{
						fc_tx_calculate_burst_size(self);
						/* Still have burst to send */
						self->refreshData(self);
						fc_send_one_burst(self);
					}
					else
					{
#if DBG_FC_TX_INFOS_ENABLED
		                sprintf(string, "[FC] [exampleEventCallback] Nop\n"); /* To remove/make explicit */
		            	xQueueSend(pPrintQueue, string, 0);
#endif
					}
            	}
            	else
            	{
#if DBG_FC_TX_INFOS_ENABLED
	                sprintf(string, "[FC] [exampleEventCallback] Not the awaited ACK\n");
	            	xQueueSend(pPrintQueue, string, 0);
#endif
            	}
            }
            else if (event == FC_BURST_NACK)
            {
                unsigned int packetNacked = val;
                if ((val <= self->packetSent) || (val > (self->packetsPending + self->packetSent)))
                {
#if DBG_FC_TX_INFOS_ENABLED
	                sprintf(string, "[FC] [exampleEventCallback] Error: NACK indicates packet number %d which is out of range\n", packetNacked);
	            	xQueueSend(pPrintQueue, string, 0);
#endif
                }
                else
                {
                    /* NACK is valid */
                    self->offset = (self->packetsPending + self->packetSent) - packetNacked +1;
#if DBG_FC_TX_INFOS_ENABLED
	                sprintf(string, "[FC] [exampleEventCallback] Offset is: %d\n", self->offset);
	            	xQueueSend(pPrintQueue, string, 0);
#endif
                    //self->packetSent      = packetNacked;
                    //self->packetsPending  = packetNacked;
                    fc_send_one_burst(self);
                }
            }

            break;
        }
        case FC_STATE_OVER:
        {
#if DBG_FC_TX_INFOS_ENABLED
            sprintf(string, "[FC] [exampleEventCallback] Transfer is over\n");
        	xQueueSend(pPrintQueue, string, 0);
#endif
            break;
        }
        default:
        {
#if DBG_FC_TX_INFOS_ENABLED
            sprintf(string, "[FC] [exampleEventCallback] Not handled yet\n");
        	xQueueSend(pPrintQueue, string, 0);
#endif
            break;
        }
    }
    return 1;

}


/* Save as a template */
int refreshTxHandlerDataImpl(struct fc_tx_handler_t * self)
{

#if DBG_FC_TX_INFOS_ENABLED
    sprintf(string, "[FC] [refreshTxHandlerDataImpl] Got to copy %d bytes, from %d to %d\n", self->bytesToProcess, self->startIndex, self->endIndex);
	xQueueSend(pPrintQueue, string, 0);
#endif
    self->bufferElems = self->bytesToProcess;

    unsigned int size   = self->endIndex-self->startIndex;
    unsigned int offset = self->startIndex;

    //memcpy(self->buffer, bigBuffer+offset, size);

    return 1;
}

/* The function initialize the handler */
void fc_tx_init(fc_tx_handler_t * self)
{

	if ( (!self->totalBytes) || (!self->payloadSize) || (!self->burstSize))
    {
        /* TODO: Assert */
        /* Error */
    }

    unsigned int fictiveBurstSize = 0;

    self->totalPackets = self->totalBytes / self->payloadSize;

	if ( self->totalBytes % self->payloadSize)
	{
		self->totalPackets++;
	}

    self->totalBurst = self->totalPackets/self->burstSize;

	if ( self->totalPackets % self->burstSize)
	{
		self->totalBurst++;
	}

    if (self->totalPackets < self->burstSize)
    {
        fictiveBurstSize = self->totalPackets;
    }
    else
    {
        fictiveBurstSize = self->burstSize;
    }


    //printf("Have to send %d packets \n", self->totalPackets);

#if DBG_FC_TX_INFOS_ENABLED
    /* Mimic an init Packet */
    sprintf(string, "[FC] [fc_tx_init] Init packet :: [HEADER][DA_SA][FC=0x02][DSAP=%02x SSAP=%02x][[N_PACKETS=%d][BURST=%d][TAIL]\n",
    						(unsigned char) (self->ServiceID>>8), (unsigned char) self->ServiceID, self->totalPackets, fictiveBurstSize);
	xQueueSend(pPrintQueue, string, 0);
#endif
    unsigned char buffer[32];
    unsigned int  length = 0;

    fc_parse_frame_flow_init(self->destination, self->source, self->ServiceID, self->totalPackets, &fictiveBurstSize, buffer, &length);
    if (self->txData(buffer, length))
    {
    	/* Sent init frame */
    }

    self->currentState = FC_STATE_WAITING_INIT_PACKET_ANSWER;
}

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
