/**
 * @file fc_rx.h
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */


#ifndef TABLET_COM_PROTOCOL_FC_RX_H_
#define TABLET_COM_PROTOCOL_FC_RX_H_

typedef enum {
	FC_IDLE = 0,
	FC_RETRIEVING_JSON,
	FC_RETRIEVING_RAW,
	FC_RETIEVING_MEAS_LIST
} FC_STATE;

typedef struct {
	 void (*dataRxCallback)(unsigned char * buffer, unsigned int length);	/* callback called when a data packet is received */
	 void (*dataRxCpltCallback)(void);	/* callback called when the reception is totally over */

	 unsigned int PacketsExpected;
	 unsigned int PacketsReceived;
	 unsigned int Burst;
	 unsigned int ACK_Counter;
	 unsigned int Enabled;
	 FC_STATE FC_State;

} fc_rx_handler_t;

#endif /* TABLET_COM_PROTOCOL_FC_RX_H_ */
