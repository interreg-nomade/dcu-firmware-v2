/*
 * frames.h
 *
 *  Created on: Jan 9, 2019
 *      Author: aclem
 */

#ifndef CLOUD_PROT_FC_FRAMES_H_
#define CLOUD_PROT_FC_FRAMES_H_

int fc_parse_frame_flow_init(unsigned char destination, unsigned char source, unsigned short service,
								unsigned int packets, unsigned char burstSize, unsigned char * dest, unsigned int * destlength);

int fc_parse_frame_ack(unsigned char destination, unsigned char source,
								unsigned int packets, unsigned char * dest);

int fc_parse_frame_nack(unsigned char destination, unsigned char source,
								unsigned int packets, unsigned char * dest);

#endif /* CLOUD_PROT_FC_FRAMES_H_ */
