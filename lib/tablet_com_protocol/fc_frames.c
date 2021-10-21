/*
 * frames.c
 *
 *  Created on: Jan 9, 2019
 *      Author: aclem
 */


#include "fc_frames.h"
#include "defines.h"
#include "string.h"


int fc_parse_frame_flow_init(unsigned char destination, unsigned char source, unsigned short service,
								unsigned int packets, unsigned char burstSize, unsigned char * dest, unsigned int * destlength)
{
	if ((!dest) || !(destlength))
	{
		return 0;
	}
	memset(dest, 0, 16);

	dest[0]  = CPL_START_DELIMITER;
	dest[1]  = 0x0A;
	dest[2]  = 0x0A;
	dest[3]  = CPL_START_DELIMITER;
	dest[4]  = destination;
	dest[5]  = source;
	dest[6]  = FLOW_CONTROL_FUNCTION_CODE_INIT;
	dest[7]  = (service >> 8) & 0xff;
	dest[8]  = (unsigned char) service & 0xff;
	dest[9]  = (packets >> 24) & 0xff;
	dest[10] = (packets >> 16) & 0xff;
	dest[11] = (packets >> 8 ) & 0xff;
	dest[12] = (packets      ) & 0xff;
	dest[13] = burstSize;
	dest[14] = 0; /* FCs field */
	for (unsigned int i = 0; i<14; i++)
	{
		dest[14] += dest[i];
	}
	dest[15] = CPL_END_DELIMITER;	/* tail */
	*destlength = 16;

	return 16;
}

int fc_parse_frame_ack(unsigned char destination, unsigned char source,
								unsigned int packets, unsigned char * dest)
{
	if ((!dest))
	{
		return 0;
	}
	memset(dest, 0, 14);

	dest[0]  = CPL_START_DELIMITER;
	dest[1]  = 0x08;
	dest[2]  = 0x08;
	dest[3]  = CPL_START_DELIMITER;
	dest[4]  = destination;
	dest[5]  = source;
	dest[6]  = FLOW_CONTROL_FUNCTION_CODE_ACK;
	dest[7]  = FLOW_CONTROL_ACK_VALUE;
	dest[8]  = (packets >> 24) & 0xff;
	dest[9]  = (packets >> 16) & 0xff;
	dest[10] = (packets >> 8 ) & 0xff;
	dest[11] = (packets      ) & 0xff;
	for (unsigned int i = 4; i<12; i++)
	{
		dest[12] += dest[i];
	}
	dest[13] = CPL_END_DELIMITER;

	return 14;
}

int fc_parse_frame_nack(unsigned char destination, unsigned char source,
								unsigned int packets, unsigned char * dest)
{
	if ((!dest))
	{
		return 0;
	}
	memset(dest, 0, 14);

	dest[0]  = CPL_START_DELIMITER;
	dest[1]  = 0x08;
	dest[2]  = 0x08;
	dest[3]  = CPL_START_DELIMITER;
	dest[4]  = destination;
	dest[5]  = source;
	dest[6]  = FLOW_CONTROL_FUNCTION_CODE_ACK;
	dest[7]  = FLOW_CONTROL_NACK_VALUE;
	dest[8]  = (packets >> 24) & 0xff;
	dest[9]  = (packets >> 16) & 0xff;
	dest[10] = (packets >> 8 ) & 0xff;
	dest[11] = (packets      ) & 0xff;
	for (unsigned int i = 4; i<12; i++)
	{
		dest[12] += dest[i];
	}
	dest[13] = CPL_END_DELIMITER;

	return 14;
}


