/**
 *
 *
 *
 * @file streaming.c
 * @brief Streaming part of the Tablet Communication Protocol
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */


/* Standard libraries */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Custom libraries */
#include "parser.h"
#include "streaming.h"

/* Contains the definitions of the protocol */
#include "defines.h"

#include "usart.h"
#include "app_init.h" // to declare QueueHandle_t

#define PRINTF_STREAMING_UPLINK_DBG 1

/* To move in header later */
void stream_parse_packet(streaming_packet_t * packet, unsigned char * dest, unsigned int *destlen);

extern char string[];
extern QueueHandle_t pPrintQueue;


/* Function to call at bootup */
void stream_init(streamingPort_handler_t * pHandler)
{
	memset(pHandler, 0, sizeof(streamingPort_handler_t));
}

/* Function to call once a start streaming data is received */
void stream_setup(streamingPort_handler_t * pHandler)
{
//#if PRINTF_STREAMING_UPLINK_DBG
//	xQueueSend(pPrintQueue, "[streaming] [stream_setup] Started.\n", 0);
//#endif
  /* Clarify if we reset the counters */
  pHandler->ourCycleCounter   = 0;
  pHandler->theirCycleCounter = 0;
  app_rtc_set_cycle_counter(1); 												/* set cycle counter to 1 */

  /* TODO check packetnumber and nbytes  infos are correct through defines */
  if (!pHandler->buffer)
  {
    //free(pHandler->buffer);
#if PRINTF_STREAMING_UPLINK_DBG
	xQueueSend(pPrintQueue, "[streaming] [stream_setup] !pHandler->buffer.\n", 0);
#endif
  }
  /* allocate memory */
  //pHandler->buffer = (unsigned char*) malloc(STREAMING_BUFFER_SIZE);
}

/* Function to call once a start streaming data is received */
void stream_send(streamingPort_handler_t * pHandler)
{
  /*
  pHandler->ourCycleCounter = 0;
  pHandler->theirCycleCounter = 0;
  pHandler->streamingPacketsNumber = pMsg->DU[0];
  pHandler->streamingPacketsbytes = (pMsg->DU[1] << 8) | pMsg->DU[2];
  */
  pHandler->comState = streamingPort_COM_BUSY; /* Pass state of resource as BUSY, if resource is used by several tasks use of a mutex must be considered */
  streaming_packet_t packet;
  unsigned int end   				= 0;
  unsigned int nPackets    			= 0;
  unsigned int nBytesToProcess  	= pHandler->elements;
  unsigned int nBytesProcessed  	= 0;
  unsigned int packetStartIndex 	= 0;
  unsigned int bytesForThisPacket 	= 0;
  memset(&packet, 0, sizeof(streaming_packet_t));
  end = pHandler->elements;
  nPackets = end/pHandler->bytesPerPackets + 1; /* Calculate the number of packets we have to send */
  for (unsigned int i = 0; i < nPackets; i++)
  {
	if ((i+1) == nPackets)
	{
	  bytesForThisPacket = nBytesToProcess - nBytesProcessed; /* Send end - start bytes */
      nBytesProcessed += bytesForThisPacket;
	}
    else
    {
      bytesForThisPacket = pHandler->bytesPerPackets;
      nBytesProcessed   += bytesForThisPacket;
    }
    packetStartIndex     = nBytesProcessed-bytesForThisPacket;
	packet.destination 	 = pHandler->destination | 0x80;
	packet.source		 = pHandler->source;
	packet.fc			 = FLOW_CONTROL_FUNCTION_CODE_NOFLOW;
	packet.dsap			 = pHandler->dsap;
	packet.ssap 		 = pHandler->ssap;
	packet.cycleCounter  = pHandler->ourCycleCounter;
	packet.packetNumber  = i+1;
	packet.payloadLength = bytesForThisPacket; /* Calculate the payload size we have to send */
	packet.payload       = pHandler->buffer+packetStartIndex;
	unsigned char buf[256];
	memset(buf, 0, 256);
	unsigned int el = 0;
	if (nPackets > 1)
	{
	  if (i+1 == nPackets)
	  {
	    //This is the last one of the burst so we add 0X80
		//packet.payload[bytesForThisPacket] = 0x80;
		packet.packetStatus = 0x80;
	  }
	  else
	  {
	    // This is not the last one of the burst, others are following!
		//packet.payload[bytesForThisPacket] = 0x7F;
		packet.packetStatus = 0x7F;
	  }
	}
	else if (nPackets == 1)
	{
	  packet.packetStatus = 0x80;
	}
	stream_parse_packet(&packet, buf, &el);
	pHandler->txData(buf, el);  // this will call function UART3_WriteBytes(uint8_t * data, uint32_t n)
	pHandler->comState = streamingPort_COM_AVAILABLE; 	/* Pass state of resource as Available, if resource is used by several tasks use of a mutex must be considered */
//#if PRINTF_STREAMING_UPLINK_DBG
////    sprintf(string, "%u [streaming] [stream_send] Sending: ",(unsigned int) HAL_GetTick());
//    sprintf(string, "%u Send: ",(unsigned int) HAL_GetTick()); // shorter sentence so that all data can be printed
//	char DUString[3];
//	for (unsigned int i = 0; i<el; i++)
//	{
//	  if (strlen(string) < 147)
//	  {
//      	  sprintf(DUString, "%02X",buf[i]);
//      	  strcat(string, DUString);
//	  }
//	}
// 	strcat(string,"\n");
//	xQueueSend(pPrintQueue, string, 0);
//#endif
  }
}

int stream_process(streamingPort_handler_t * pHandler, unsigned char * buffer, unsigned int * n)
{
	if ((!pHandler) || (!buffer) || (!n))
	{
		return 0;
	}

	return 1;
}


/* load data in the streaming port handler */
int stream_load(streamingPort_handler_t * pHandler, unsigned char * buffer, unsigned int size)
{
	if ((!pHandler) || (!buffer) || (size == 0))
	{
		return 0;
	}
	if (pHandler->comState == streamingPort_COM_AVAILABLE)
	{
		/* Buffer is not in use */
		if (size < STREAMING_BUFFER_SIZE)
		{
			memcpy(pHandler->buffer, buffer, size);
			pHandler->elements = size;
			return 1;
		}
		else
		{
			//TODO
			//stream_load_several_packets(pHandler, buffer, n);
			//Will have to use several packets
		}
	}
	return 0;
}

/* load data in the streaming port handler */
int stream_load_several_packets(streamingPort_handler_t * pHandler, unsigned char * buffer, unsigned int * n)
{
	if ((!pHandler) || (!buffer) || (!n))
	{
		return 0;
	}
	if (pHandler->comState == streamingPort_COM_AVAILABLE)
	{
		/* Buffer is not in use */
		/* todo: considere use of a mutex */
		if ((*n) < STREAMING_BUFFER_SIZE)
		{
			//buffer[*n] = 0x80;
			//(*n)++;
			return 1;
		}
		else
		{
			memcpy(pHandler->buffer, buffer, *n);
			pHandler->elements = *n;
			//Will have to use several packets
			//TODO
		}
	}
	return 0;
}

void stream_parse_packet(streaming_packet_t * packet, unsigned char * dest, unsigned int *destlen)
{
    dest[0] = 0x68;
    dest[1] = packet->payloadLength + 11;
 	dest[2] = packet->payloadLength + 11;
    dest[3] = 0x68;
    dest[4] = packet->destination;
    dest[5] = packet->source;
    dest[6] = packet->fc; /* FC */
    dest[7] = packet->dsap;
	dest[8] = packet->ssap;
    dest[9]  = packet->cycleCounter  >> 24   & 0xff;
    dest[10] = packet->cycleCounter  >> 16   & 0xff;
    dest[11] = packet->cycleCounter  >> 8  	 & 0xff;
    dest[12] = packet->cycleCounter 		 & 0xff;
    dest[13] = packet->packetNumber;
    for (unsigned int i = 0; i<packet->payloadLength+14; i++)
    {
        dest[i+14] = packet->payload[i];
    }
	dest[14+packet->payloadLength] = packet->packetStatus;
    unsigned char checksum = 0;
    for (unsigned int i = 4; i < (packet->payloadLength+15); i++)
    {
        checksum += dest[i];
    }
	dest[15+packet->payloadLength] = checksum;
	dest[16+packet->payloadLength] = 0x16;

	*destlen = 17+packet->payloadLength;
}

void stream_tester_output(unsigned char * buffer, unsigned int len)
{
	for (unsigned int i = 0; i<len; i++)
	{
		printf(" 0x%02x ", buffer[i]);
	}
	printf("\n");
}

void stream_tester(void)
{
	streamingPort_handler_t port;
	cpl_msg_t msg;

	stream_init(&port);

	msg.DU[0] = 0x03;
	msg.DU[1] = 0x02;
	msg.DU[2] = 0x8a;

	stream_setup(&port);

//	printf("stream setup: number of packets per stream burst: %d, number of bytes in total: %d\n",
//														port.streamingPacketsNumber, port.streamingPacketsbytes);

	memset(port.buffer, 'A', 256);
	memset(port.buffer + 256, 'B', 256);
	memset(port.buffer + 512, 'C', 256);
	memset(port.buffer + 768, 'D', 256);

	port.txData   = stream_tester_output; // this function outputs on the printf for dbg purposes
	port.elements = 600;
	port.bytesPerPackets = STREAMING_PACKET_DEFAULT_PAYLOAD_SIZE;

	stream_send(&port);

}



#if 0
void stream_tx_calculate_burst_size(struct fc_tx_handler_t * self)
{
    unsigned int start = 0;
    unsigned int end   = 0;

    if ((self->totalPackets - self->packetSent) < self->burstSize)
    {
        start = self->burstSent     *  (self->burstSize * self->payloadSize);
        end   = self->totalBytes -1;
#if DBG_FC_TX_INFOS_ENABLED
        printf("Should point :: start:%d  end:%d \n", start, end);
        printf("Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
        printf("And copy %d bytes\n", self->burstSize * self->payloadSize);
        printf("Last burst to send\n");
#endif
        self->packetsPending = (self->burstSize % (self->totalPackets-self->packetSent) +1);
    }
    else if ((self->totalPackets - self->packetSent) == self->burstSize)
    {
        start = self->burstSent     *  (self->burstSize * self->payloadSize);
        end   = self->totalBytes -1;
#if DBG_FC_TX_INFOS_ENABLED
        printf("Should point :: start:%d  end:%d \n", start, end);
        printf("Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
        printf("And copy %d bytes\n", self->burstSize * self->payloadSize);
        printf("Packets to send exactly equal to size of one burst\n");
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
        printf("Should point :: start:%d  end:%d \n", start, end);
        printf("Should point to elem %d\n", self->burstSent *  (self->burstSize * self->payloadSize) /*- 1 * (self->burstSize * self->payloadSize)*/);
        printf("And copy %d bytes\n", self->burstSize * self->payloadSize);
#endif
    }
    self->startIndex = start;
    self->endIndex   = end;
    self->bytesToProcess = end-start+1; /* fix the offset */
    //self->offset = self->burstSize;
    self->offset = self->packetsPending;
}
#endif
