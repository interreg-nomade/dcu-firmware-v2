/* parser.c
 * @file parser.c
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 *
 *     Adapted for Nomade project: August 31, 2020 by Sarah Goossens
 *
 */
#include "parser.h"

#include <stdlib.h>
#include <string.h>

#include "../ring_buffer/ringbuffer_char.h"

#include "usart.h"
#include "app_init.h" // to declare QueueHandle_t

/* Variable definitions */
static ring_buffer_t cplRingbufRx;

/* Debug! TODO: remove once dev is finished, or macro comment */

#define CPL_SM_DBG_PRINTF 0

#define CPL_TAIL_LENGTH			2
#define CPL_HEADER_LENGTH 		4

/* Declare private functions */
static int cpl_FindSdByte(void);
static int cpl_FcsCorrect(void);
int cpl_calculate_fcs(cpl_msg_t * pCplMsg);

static int cpl_BuildHeader(void);
static int cpl_BuildBody(void);
static int cpl_FcsCorrect(void);
static int cpl_BodyPartPresent(void);
static int cpl_HeaderPartPresent(void);
static int cpl_TailPartPresent(void);
static void cpl_RstTimeout(unsigned int * var);
static int cpl_TimedOut(unsigned int * var);


static cpl_msg_t tmpCplMsg;

extern char string[];
extern QueueHandle_t pPrintQueue;

void cpl_RxHandler(char c)
{
    // Queue in ring buffer the rx'd byte
    ring_buffer_queue(&cplRingbufRx, c);
}

unsigned int cpl_RxBufferSize(void)
{
	return ring_buffer_num_items(&cplRingbufRx);
}

void cpl_init()
{
    ring_buffer_init(&cplRingbufRx);
    cpl_rst_msg(&tmpCplMsg);
}

void cpl_rst_msg(cpl_msg_t * pCplMsg)
{
    if (pCplMsg != NULL)
    {
        pCplMsg->FC = 0;
        pCplMsg->FCS = 0;
        pCplMsg->destinationAddress = 0;
        pCplMsg->sourceAddress = 0;
        pCplMsg->lenght = 0;
        pCplMsg->destinationService = 0;
        pCplMsg->sourceService = 0;

        memset(pCplMsg->DU, 0, MAX_CPL_PAYLOAD_LENGTH);
    }
}

CPL_OP_RESULT cpl_prot_decoder(cpl_msg_t * pCplMsg,
		uint32_t actualTime)
{
	static uint32_t lastTime = 0;
    static CPL_PARSER_STATE smState = CPL_IDLE;

    CPL_OP_RESULT opRes = CPL_NO_MSG;
//    static unsigned int timeout = 0;

    switch (smState)
    {
        case CPL_IDLE:
        {
    		/* IDLE state of the parser */
            if (cpl_FindSdByte())
            {
#if CPL_SM_DBG_PRINTF
            	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_IDLE] found Start Delimiter.\n",(unsigned int) HAL_GetTick());
                xQueueSend(pPrintQueue, string, 0);
#endif
    			/* Put the temporary CPL message in a known state */
                cpl_rst_msg(&tmpCplMsg);
    			/* Take the actual time as reference for the timeout */
    			lastTime = actualTime;
    			/* Go to the CPL building header state */
                smState = CPL_BUILDING_HEADER;
//                cp_prot_decoder(pCplMsg);
    			/* No break to Flow through */
            }
            else
            {
                opRes = CPL_NO_MSG;
                break;
            }
        } // @suppress("No break at end of case")
        /* no break */

    	/* Building header state of the parser */
        case CPL_BUILDING_HEADER:
        {
#if CPL_SM_DBG_PRINTF
            sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] Start building header.\n",(unsigned int) HAL_GetTick());
            xQueueSend(pPrintQueue, string, 0);
#endif
    		/* Check if the header part is present */
        	if (cpl_HeaderPartPresent())
            {
#if CPL_SM_DBG_PRINTF
        		sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] CPL header part present.\n",(unsigned int) HAL_GetTick());
        	    xQueueSend(pPrintQueue, string, 0);
#endif
    			/* if the header is present */
    			/* Update the timeout reference with the actual time */
    			lastTime = actualTime;
    			/* Build the header part of the message */
                if (cpl_BuildHeader())
                {
                    /* Could build a header */
#if CPL_SM_DBG_PRINTF
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] LEngth is correctly repeated, LE=%d\n",(unsigned int) HAL_GetTick(), tmpCplMsg.lenght);
            	    xQueueSend(pPrintQueue, string, 0);
#endif
    				/* Pass the parser to building body state */
                	smState = CPL_BUILDING_BODY;
                }
                else
                {
                    /* Invalid header */
#if CPL_SM_DBG_PRINTF
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] Less than 2 elements in the buffer\n",(unsigned int) HAL_GetTick());
            	    xQueueSend(pPrintQueue, string, 0);
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] Back to Idle, LE=%d\n",(unsigned int) HAL_GetTick(), tmpCplMsg.lenght);
            	    xQueueSend(pPrintQueue, string, 0);
#endif
                    smState = CPL_IDLE;
                    opRes = CPL_NO_MSG;
                    break;
                }
            }
            else
            {
    			/* If no header part of the message is present */
#if CPL_SM_DBG_PRINTF
            	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] Less then 3 items in buffer\n",(unsigned int) HAL_GetTick());
        	    xQueueSend(pPrintQueue, string, 0);
#endif
    			/* Check for the timeout */
        		if((actualTime - lastTime >= CPL_TIMEOUT_REF))
                {
#if CPL_SM_DBG_PRINTF
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_HEADER] Time out in building header, back to CPL_IDLE state.\n",(unsigned int) HAL_GetTick());
            	    xQueueSend(pPrintQueue, string, 0);
#endif
                	smState = CPL_IDLE;
    				opRes = CPL_NO_MSG;
                }
                break;
            }
        } // @suppress("No break at end of case")
        /* no break */

    	/* Building body part of the parser */
        case CPL_BUILDING_BODY:
        {
#if CPL_SM_DBG_PRINTF
            sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_BODY] start building body.\n",(unsigned int) HAL_GetTick());
    	    xQueueSend(pPrintQueue, string, 0);
#endif
            if (cpl_BodyPartPresent())
            {
    			/* If the body part is present */
    			/* Update the timeout reference with the actual time */
#if CPL_SM_DBG_PRINTF
                sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_BODY] [cpl_BodyPartPresent] %u elements in ringbuffer, >= %d LEngth.\n",(unsigned int) HAL_GetTick(),(unsigned int) ring_buffer_num_items(&cplRingbufRx),(tmpCplMsg.lenght +5));
        	    xQueueSend(pPrintQueue, string, 0);
#endif
                if (cpl_BuildBody())
                {
    				/* Correct body part */
                	#if CPL_SM_DBG_PRINTF
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILD_BODY] Going to check frame\n",(unsigned int) HAL_GetTick());
            	    xQueueSend(pPrintQueue, string, 0);
#endif
    				/* Going to checking the frame */
                    smState = CPL_CHECKING_FRAME;
                }
                else
                {
#if CPL_SM_DBG_PRINTF
                	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILD_BODY] Invalid body, back to Idle\n",(unsigned int) HAL_GetTick());
            	    xQueueSend(pPrintQueue, string, 0);
#endif
    				/* Invalid body */
    				/* Go back to the IDLE state of the parser */
                	smState = CPL_IDLE;
                    opRes = CPL_NO_MSG;
                    break;
                }
            }
            else
            {
#if CPL_SM_DBG_PRINTF
            	sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_BODY] [cpl_BodyPartPresent = 0] CPL body part not present: expected %d elements, but only %u elements available.\n",(unsigned int) HAL_GetTick(),(tmpCplMsg.lenght+5),(unsigned int) ring_buffer_num_items(&cplRingbufRx));
        	    xQueueSend(pPrintQueue, string, 0);
#endif
        		if((actualTime - lastTime >= CPL_TIMEOUT_REF))
                {
#if CPL_SM_DBG_PRINTF
                    sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_BUILDING_BODY] [cpl_BodyPartPresent = 0] Time out in building body, smState = CPL_IDLE.\n",(unsigned int) HAL_GetTick());
            	    xQueueSend(pPrintQueue, string, 0);
#endif
                    smState = CPL_IDLE;
    				opRes = CPL_NO_MSG;
                    break;
                }
    			smState = CPL_BUILDING_BODY;
            	break;
            }
        }
        /* no break */

        case CPL_CHECKING_FRAME:
        {
    		unsigned int counter = 0;
    		if(cpl_TailPartPresent())
    		{
    			/* if the tail of the message is present */
    			/* Update the timeout reference with the actual time */
    			lastTime = actualTime;
    			if (cpl_FcsCorrect())
    			{
    				/* If the FCS is correct */
    				/* Building ID */
    				tmpCplMsg.SERVICE = (tmpCplMsg.destinationService << 8) | tmpCplMsg.sourceService;
	#if CPL_SM_DBG_PRINTF
					sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] SERVICE = %d.\n",(unsigned int) HAL_GetTick(),tmpCplMsg.SERVICE);
	        	    xQueueSend(pPrintQueue, string, 0);
	#endif
					/* If the packet is a FC packet: get the packet number */
					//todo: use sap bitfields.
					if (tmpCplMsg.FC == FLOW_CONTROL_FUNCTION_CODE_DATA) /* 0x04 */
					{
						tmpCplMsg.packetNumber = ((tmpCplMsg.destinationService << 24) |
												 (tmpCplMsg.sourceService << 16)  |
												 (tmpCplMsg.DU[0] << 8 )		  |
												 (tmpCplMsg.DU[1]));
	#if CPL_SM_DBG_PRINTF
						sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] [FC = FLOW_CONTROL_FUNCTION_CODE_DATA] Packet Number = %d.\n",(unsigned int) HAL_GetTick(),tmpCplMsg.packetNumber);
		        	    xQueueSend(pPrintQueue, string, 0);
	#endif
					}
					else if (tmpCplMsg.FC == FLOW_CONTROL_FUNCTION_CODE_ACK)  /* 0x06 */
					{
						if (tmpCplMsg.destinationService == FLOW_CONTROL_NACK_VALUE)
						{
							tmpCplMsg.flowCode = FLOW_CONTROL_NACK_VALUE;
						}
						else if (tmpCplMsg.destinationService == FLOW_CONTROL_ACK_VALUE)
						{
							tmpCplMsg.flowCode = FLOW_CONTROL_ACK_VALUE;
						}
						tmpCplMsg.packetNumber = ((tmpCplMsg.sourceService << 24) |
												 (tmpCplMsg.DU[0] << 16)         |
												 (tmpCplMsg.DU[1] << 8 )		 |
												 (tmpCplMsg.DU[2]));
#if CPL_SM_DBG_PRINTF
						sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] [FC = FLOW_CONTROL_FUNCTION_CODE_ACK] Packet Number = %d.\n",(unsigned int) HAL_GetTick(),tmpCplMsg.packetNumber);
		        	    xQueueSend(pPrintQueue, string, 0);
#endif
					}
//					else if (tmpCplMsg.FC == FLOW_CONTROL_FUNCTION_CODE_NOFLOW)  /* 0x00 */
//					{
//	#if CPL_SM_DBG_PRINTF
//						sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] [FC = case FLOW_CONTROL_FUNCTION_CODE_NOFLOW] \n",(unsigned int) HAL_GetTick());
//	 	       	    	xQueueSend(pPrintQueue, string, 0);
//	#endif
//						tmpCplMsg.payloadLength = tmpCplMsg.lenght - 5;
//
//					}

					*pCplMsg = tmpCplMsg; /* Copy the contents of the tmpCplMsg in the reference */
#if CPL_SM_DBG_PRINTF
					sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] Correct frame ID = %04x\n",(unsigned int) HAL_GetTick(), tmpCplMsg.SERVICE);
	        	    xQueueSend(pPrintQueue, string, 0);
//#endif
					sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Correct] Contents of received Package with 0x%X bytes:\n",(unsigned int) HAL_GetTick(),tmpCplMsg.lenght);
	        	    xQueueSend(pPrintQueue, string, 0);
	                sprintf(string, " ");
	        	      char DUString[3];
		        	  for (unsigned int i = 0; i<tmpCplMsg.lenght; i++)
	        	      {
	        	    	  if (strlen(string) < 147)
	        	    	  {
	            	    	sprintf(DUString, "%02X",tmpCplMsg.DU[i]);
	            	    	strcat(string, DUString);
	        	    	  }
	        	      }
	         	      strcat(string,"\n");
	        	      xQueueSend(pPrintQueue, string, 0);
#endif
					opRes = CPL_CORRECT_FRAME;
					smState = CPL_IDLE;
					break;
				}
				else
				{
#if CPL_SM_DBG_PRINTF
					sprintf(string,"%u [PARSER] [cp_prot_decoder] [smState = CPL_CHECKING_FRAME] [Frame Check Sequence Not Correct] Incorrect Frame, back to Idle.\n",(unsigned int) HAL_GetTick());
	        	    xQueueSend(pPrintQueue, string, 0);
#endif
					/* If the FCS is incorrect go back to the IDLE state of the parser */
					smState = CPL_IDLE;
					/* Return the result as incorrect */
					opRes = CPL_INCORRECT_FRAME;
					break;
				}
    		}
    		else
    		{
    			/* if the tail part is not present check for the timeout */
    			if((actualTime - lastTime >= CPL_TIMEOUT_REF))
    			{
    				opRes = CPL_NO_MSG;
    				smState = CPL_IDLE;
    				break;
    			}
    			break;
    		}
        }
    }
    return opRes;
}


int cpl_calculate_fcs(cpl_msg_t * pCplMsg)
{
    if ((pCplMsg == NULL) || (pCplMsg->lenght > MAX_CPL_PAYLOAD_LENGTH))
    {
        return 0;
    }

    pCplMsg->FCS = 0;
    pCplMsg->FCS  = pCplMsg->destinationAddress;
    pCplMsg->FCS += pCplMsg->sourceAddress;
    pCplMsg->FCS += pCplMsg->destinationService;
    pCplMsg->FCS += pCplMsg->sourceService;
    pCplMsg->FCS += pCplMsg->FC;

    for (unsigned int i = 0; i < (pCplMsg->lenght-5); i++)
    {
        pCplMsg->FCS += pCplMsg->DU[i];
    }

    return 1;
}

static int cpl_FindSdByte(void)
{
    char rxdByte;
    rxdByte = 0;

    while (!ring_buffer_is_empty(&cplRingbufRx))
    {
        ring_buffer_dequeue(&cplRingbufRx, &rxdByte);
        if (rxdByte == CPL_START_DELIMITER)
        {
            return 1;
        }
    }
    return 0;
}

static int cpl_BuildHeader(void)
{
    char rxdByte[2];

    memset(rxdByte, 0, 2);

    if (ring_buffer_num_items(&cplRingbufRx) >= 3)
    {
		ring_buffer_dequeue(&cplRingbufRx, &rxdByte[0]);
		ring_buffer_dequeue(&cplRingbufRx, &rxdByte[1]);
        if (rxdByte[0] == rxdByte[1])
        {
            /* LEngth is correctly repeated */
            tmpCplMsg.lenght = rxdByte[0];
			ring_buffer_dequeue(&cplRingbufRx, &rxdByte[1]);
			if(rxdByte[1] == CPL_START_DELIMITER)
			{
				return 1;
			}
        }
    }
    else
    {
        /* Less than 2 elems in the buffer */
    }
    return 0;
}

static int cpl_BuildBody(void)
{
	if(ring_buffer_num_items(&cplRingbufRx) >= tmpCplMsg.lenght)
	{
		ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.destinationAddress);
		ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.sourceAddress);
		ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.FC);
		ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.destinationService);
		ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.sourceService);

		tmpCplMsg.payloadLength = tmpCplMsg.lenght - 5; /* DU = BODY LENGTH - (DALen + SALen + FCLen + DSAPLen + SSAPLen */

		for(unsigned int i = 0; i < tmpCplMsg.payloadLength; i++)
		{
			ring_buffer_dequeue(&cplRingbufRx, (char*)&tmpCplMsg.DU[i]);
		}

		return 1;
	}

    return 0;
}

static int cpl_FcsCorrect(void)
{
	char retrievedFcs;
	char retrievedEd;

	if(ring_buffer_num_items >= CPL_TAIL_LENGTH)
	{
		ring_buffer_dequeue(&cplRingbufRx, &retrievedFcs);
		ring_buffer_dequeue(&cplRingbufRx, &retrievedEd);

		if (retrievedEd == CPL_END_DELIMITER)
		{
			cpl_calculate_fcs(&tmpCplMsg);

			if(retrievedFcs == tmpCplMsg.FCS)
			{
				/* The calculated FCS is equal to FCS received */
				return 1;
			}
			else
			{
				/* The calculated FCS is not equal to FCS received */
				return 0;
			}
		}
		else
		{
			/* No end delimiter */
			return 0;
		}
	}

    return 0;
}

int cpl_buildFrame(cpl_msg_t * Msg, unsigned int * cx, char * buffer)
{
    if ((Msg == NULL) || (buffer == NULL) || (cx == NULL))
    {
        return 0;
    }

    /* Header */
    buffer[0] = CPL_START_DELIMITER;
    buffer[1] = Msg->lenght;
    buffer[2] = Msg->lenght;
    /* Body */
    buffer[3] = CPL_START_DELIMITER;

    buffer[4] = Msg->destinationAddress;
    buffer[5] = Msg->sourceAddress;

    buffer[6] = Msg->FC;

    buffer[7] = Msg->destinationService;
    buffer[8] = Msg->sourceService;

    for (unsigned int i = 0; i < Msg->payloadLength; i++)
    {
        buffer[i + 9] = Msg->DU[i];
    }

    /* Calculate the frame check sequence */
    cpl_calculate_fcs(Msg);

    buffer[9 + Msg->payloadLength] = Msg->FCS;

    buffer[10 + Msg->payloadLength] = CPL_END_DELIMITER;

    *cx = 11 + Msg->payloadLength;

    return 1;
}

//void test(unsigned int * a)
//{
//    *a = 5555;
//}

static int cpl_BodyPartPresent(void)
{
    /* +5 because we count LE, LEr, SD, FCS, ED */
    if (ring_buffer_num_items(&cplRingbufRx) >= (tmpCplMsg.lenght))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static int cpl_TailPartPresent(void)
{
	if(ring_buffer_num_items(&cplRingbufRx) >= CPL_TAIL_LENGTH)
	{
		return 1;
	}

	return 0;
}


static int cpl_TimedOut(unsigned int * var)
{
    *var = *var +1;
    if (*var >= CPL_TIMEOUT_REF)
    {
#if CPL_SM_DBG_PRINTF
        sprintf(string, "%u [PARSER] [cpl_TimedOut] %s TIMEOUT\n",(unsigned int) HAL_GetTick(), __func__);
	    xQueueSend(pPrintQueue, string, 0);
#endif
        *var = 0;
        return 1;
    }
    else
    {
#if CPL_SM_DBG_PRINTF
//      sprintf(string, "%u [PARSER] [cpl_TimedOut] %s No TIMEOUT\n",(unsigned int) HAL_GetTick(), __func__);
//	    xQueueSend(pPrintQueue, string, 0);
#endif
        return 0;
    }
}

static void cpl_RstTimeout(unsigned int * var)
{
    *var = 0;
}

static int cpl_HeaderPartPresent(void)
{
    /* +5 because we count LE, LEr, SD, FCS, ED */
	if (ring_buffer_num_items(&cplRingbufRx) >= CPL_HEADER_LENGTH - 1)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void cpl_test_send_dummy(void)
{

    cpl_msg_t CplTxMsg;
    unsigned int uniqueCxn;
    unsigned char buffer[MAX_CPL_FRAME_LENGTH];

    cpl_rst_msg(&CplTxMsg);
    memset(buffer, 0, MAX_CPL_FRAME_LENGTH);

    /* Build */
    CplTxMsg.lenght             = 0x09;
    CplTxMsg.destinationAddress = 0x00;
    CplTxMsg.sourceAddress      = 0x01;
    CplTxMsg.FC                 = 0x02;
    CplTxMsg.destinationService = 0x10;
    CplTxMsg.sourceService      = 0x20;
    CplTxMsg.payloadLength      = 0x04;
    CplTxMsg.DU[0]              = 0x01;
    CplTxMsg.DU[1]              = 0x02;
    CplTxMsg.DU[2]              = 0x03;
    CplTxMsg.DU[3]              = 0x04;

    cpl_buildFrame(&CplTxMsg,( unsigned int* ) &uniqueCxn, (char* ) buffer);

    HAL_UART_Transmit(&huart3, buffer, uniqueCxn, 0xfff);
}

void cpl_build_and_tx_buffer(unsigned short id, char * buf, unsigned int cx)
{
    if (buf == NULL)
    {
        return;
    }
    if (cx >= 254)
    {
        return;
    }

    cpl_msg_t CplTxMsg;
    unsigned int uniqueCxn;
    unsigned char buffer[MAX_CPL_FRAME_LENGTH];

    cpl_rst_msg(&CplTxMsg);
    memset(buffer, 0, MAX_CPL_FRAME_LENGTH);

    /* Build */
    CplTxMsg.lenght             = 0x04 + cx;
    CplTxMsg.destinationAddress = 0x00;
    CplTxMsg.sourceAddress      = 0x01;
    CplTxMsg.FC                 = 0x02;
    CplTxMsg.destinationService = id >> 8;
    CplTxMsg.sourceService      = id;
    CplTxMsg.SERVICE = id;
    CplTxMsg.payloadLength      = cx;
    for (unsigned int i = 0; i < cx; i++)
    {
        CplTxMsg.DU[i] = buf[i];
    }

    cpl_buildFrame(&CplTxMsg, (unsigned int*) &uniqueCxn, (char *)buffer);

    HAL_UART_Transmit(&huart3, buffer, uniqueCxn, 0xfff);
}

int cpl_buildFrameNoPayload(cpl_msg_t * Msg, unsigned int * cx, char * buffer)
{
	if ((Msg == NULL) || (buffer == NULL) || (cx == NULL))
	{
		return 0;
	}

	/* Header */
	buffer[0] = CPL_START_DELIMITER;
	buffer[1] = Msg->lenght;
	buffer[2] = Msg->lenght;
	/* Body */
	buffer[3] = CPL_START_DELIMITER;

	buffer[4] = Msg->destinationAddress;
	buffer[5] = Msg->sourceAddress;

	buffer[6] = Msg->FC;

	buffer[7] = Msg->destinationService;
	buffer[8] = Msg->sourceService;

	/* Calculate the frame check sequence */
	cpl_calculate_fcs(Msg);

	buffer[9] = Msg->FCS;

	buffer[10] = CPL_END_DELIMITER;

	*cx = 11;

	return 1;
}
