/**
 * @file streaming.h
 * @brief Streaming part of the Tablet Communication Protocol
 * @author Alexis.C, Ali O.
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#ifndef CLOUD_PROT_STREAMING_H_
#define CLOUD_PROT_STREAMING_H_

#define STREAMING_BUFFER_SIZE 		2048
#define STREAMING_PACKET_MAX_SIZE 	256
#define STREAMING_PACKET_DEFAULT_PAYLOAD_SIZE 239

typedef enum {
	streamingPort_NOT_INITIALIZED = -1,
	streamingPort_DISABLED = 0,
	streamingPort_ENABLED
} streamingPort_State_t;

typedef enum {
	streamingPort_COM_AVAILABLE,
	streamingPort_COM_BUSY,
	streamingPort_COM_ERROR,

} streamingPort_ComState_t;



typedef struct {
	/* General informations */
	unsigned int streamingPacketsNumber;
	unsigned int streamingPacketsbytes;
	unsigned int bytesPerPackets;

	streamingPort_State_t state;
	streamingPort_ComState_t comState;

	unsigned int ourCycleCounter; /* Counter on mainboard side */
	unsigned int theirCycleCounter; /* Counter from the ACK received from the tablet */

	/* This function is called at every begin of a burst transfer */
	int (*txData)(uint8_t * data, uint32_t length); /* Actualize the buffer */
	/* tx function for debugging only! */
	int (*txDbgData)(unsigned char * data, unsigned int length);
	/* This function is called once the transfer is over */
	int (*txCpltCallback)(unsigned short service);

	/* Buffer */
	unsigned char buffer[1024];
	unsigned int  elements;

	unsigned char destination;
	unsigned char source;
	unsigned char dsap;
	unsigned char ssap;


} streamingPort_handler_t;

typedef struct {
	unsigned char destination;
	unsigned char source;
	unsigned char fc;
	unsigned char dsap;
	unsigned char ssap;

	unsigned int cycleCounter;
	unsigned char  packetNumber;

	unsigned char checksum;

	unsigned int payloadLength;
	unsigned char * payload; //todo dynamic

	unsigned char packetStatus;


} streaming_packet_t;

void stream_tester(void);
/* To movei n header later */
void stream_parse_packet(streaming_packet_t * packet, unsigned char * dest, unsigned int *destlen);

/* Function to call at bootup */
void stream_init(streamingPort_handler_t * pHandler);

/* Function to call once a start streaming data is received */
void stream_setup(streamingPort_handler_t * pHandler);

/* Function to call once a start streaming data is received */
void stream_send(streamingPort_handler_t * pHandler);

int stream_process(streamingPort_handler_t * pHandler, unsigned char * buffer, unsigned int * n);
/* load data in the streaming port handler */
int stream_load(streamingPort_handler_t * pHandler,
		unsigned char * buffer,
		unsigned int size);
int stream_load_several_packets(streamingPort_handler_t * pHandler, unsigned char * buffer, unsigned int * n); // niewe versie extra
void stream_tester_output(unsigned char * buffer, unsigned int len);




#endif /* CLOUD_PROT_STREAMING_H_ */
