/*
 /**
 * @file frames.c
 * @brief
 * @author Alexis.C
 * @version 0.1
 * @date March 2019
 * @project Interreg EDUCAT
 */

#include "frames.h"

void cpl_util_set_service(cpl_msg_t * Msg, short service)
{
	Msg->destinationService = (service >> 8) & 0x00ff;
	Msg->sourceService      = service 		& 0x00ff;
}
