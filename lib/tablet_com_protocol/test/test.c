/* test.c */
#include "test.h"


#include "FreeRTOS.h"
#include "task.h"
#include "cmsis_os.h"

#include "usart.h"

//#define CPL_UNIT_TES_ONLY_RX
#define CPL_UNIT_TEST_RX_TX
#define CPL_UNIT_TEST

#ifdef CPL_UNIT_TEST

char buffer[MAX_CPL_FRAME_LENGTH];
unsigned int uniqueCxn = 0;
unsigned int cxn;

/* RTOS Variables */
static osThreadId cplTestTaskHandle;

/* Cloud link protocol variables */
static cpl_msg_t CplRxMsg;
static cpl_msg_t CplTxMsg;

/* Debug variables */


void cplTestTask(void const * argument);

void cpl_init_test(void)
{
    /* Init the decoder */
    cpl_init();

    osThreadDef(testCpl, cplTestTask, osPriorityNormal, 0, 256);
    cplTestTaskHandle = osThreadCreate(osThread(testCpl), NULL);
}

#ifdef CPL_UNIT_TES_ONLY_RX
void cplTestTask(void const * argument)
{

    osDelay(3000);
    /* init */
    for (;;)
    {
        if (cp_prot_decoder(&CplRxMsg) == CPL_CORRECT_FRAME)
        {
            unsigned char c = 5;
            ;
            ;
        }
        osDelay(5);

        /* Try to send frames */
        /* Init */
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



        cpl_buildFrame(&CplTxMsg, &uniqueCxn, buffer);

        /* Trigger */
        //HAL_UART_Transmit(&huart3, buffer, uniqueCxn, 0xfff);
    }
}
#else
/* This test handles the reception and transmission, see the handshake part in the excel file pinned */
void cplTestTask(void const * argument)
{

    osDelay(3000);
    /* init */
    for (;;)
    {
        if (cp_prot_decoder(&CplRxMsg) == CPL_CORRECT_FRAME)
        {
            switch (CplRxMsg.sourceService)
            {
                case CPL_MSG_TABLET_CON_ASW:
                {
                    cxn = snprintf(buffer, 256, "Received connection answer from tablet\n");
                    HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    /* Send current configuration ID */
                    /* Send a frame to request the mode to load on: Engineer or User */
                    break;
                }

                case CPL_MSG_MODE_ASW:
                {
                    /* If Payload[0] = 1 : USER, 0: ENGINEER */
                    if (CplRxMsg.DU[0])
                    {
                        buffer[0] = 0;
                        cxn = snprintf(buffer, 256, "Received request to boot on USER mode from the tablet\n");
                        HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    }
                    else
                    {
                        buffer[0] = 0;
                        cxn = snprintf(buffer, 256, "Received request to boot on ENGINEER mode from the tablet\n");
                        HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    }
                    break;
                }

                case CPL_MSG_CONFIG_STATE_ASW:
                {
                    if (CplRxMsg.DU[0])
                    {
                        buffer[0] = 0;
                        cxn = snprintf(buffer, 256, "Received request to load a new config\n");
                        HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    }
                    else
                    {
                        buffer[0] = 0;
                        cxn = snprintf(buffer, 256, "Not loading a new config\n");
                        HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    }
                    break;
                }

                case CPL_MSG_CONFIG_STREAM_ONE_INSTR:
                {
                    buffer[0] = 0;
                    cxn = snprintf(buffer, 256, "Received config stream (one instrument at a time)\n");
                    HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    if (CplRxMsg.DU[0] == 0x10)
                    {
                        buffer[0] = 0;
                        cxn = snprintf(buffer, 256, "Configuration of an IMU RXd\nID: %d Samplerate:%d\n", CplRxMsg.DU[1]<<8 | CplRxMsg.DU[2], CplRxMsg.DU[5]);
                        HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    }
                    break;
                }
                case CPL_MSG_CONFIG_STREAM_OVER:
                {
                    buffer[0] = 0;
                    cxn = snprintf(buffer, 256, "Tablet finished to stream the config: SAVE IT.\n");
                    HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    break;
                }
                case CPL_MSG_START_MEASUREMENT_STREAM:
                {
                    buffer[0] = 0;
                    cxn = snprintf(buffer, 256, "START MEASUREMENT STREAMING!\n");
                    HAL_UART_Transmit(&huart3, buffer, cxn, 0xfff);
                    break;
                }
            }
        }
        osDelay(5);

        /* Try to send frames */
        /* Init */
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



        cpl_buildFrame(&CplTxMsg, &uniqueCxn, buffer);

        /* Trigger */
        //HAL_UART_Transmit(&huart3, buffer, uniqueCxn, 0xfff);
    }
}


#endif
#endif
