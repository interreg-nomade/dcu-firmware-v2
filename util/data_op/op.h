/**
 * @file op.h
 * @brief contains usefull functions to manipulate data; shift, transform to byte array, etc.
 */

#ifndef UTILS_DATA_OP_OP_H_
#define UTILS_DATA_OP_OP_H_

#include "stdint.h"

/**
 * @brief Transform a float to a 4 bytes array ; LSB representation
 * @param f the input value (float)
 * @param dest destination buffer pointer
 * @return 4 SUCCESS: the number of bytes written
 * @return 0 FAILURE
 */
int float_to_byte_array_lsb(float f, char * dest);



/**
 * @brief Transform a float to a 4 bytes array ; MSB representation
 * @param f the input value (float)
 * @param dest destination buffer pointer
 * @return 4 SUCCESS: the number of bytes written
 * @return 0 FAILURE
 */
int float_to_byte_array_msb(float f, char * dest);



/**
 * @brief Transform a short to a two byte array ; LSB representation
 * @param s the input value (short)
 * @param dest destination buffer pointer
 * @return 2 SUCCESS: the number of bytes written
 * @return 0 FAILURE
 */
int	short_to_byte_array_msb(short s, char * dest);



/**
 * @brief Transform a short to a two byte array ; MSB representation
 * @param s the input value (short)
 * @param dest destination buffer pointer
 * @return 2 SUCCESS: the number of bytes written
 * @return 0 FAILURE
 */
int	short_to_byte_array_lsb(short s, char * dest);


unsigned int rightshifty32_unsigned_integer_msb (unsigned int data, unsigned char *frame);
unsigned int rightshifty32_signed_integer   (int data, unsigned char *frame);

uint64_t bytes_array_to_uint64t(uint8_t* buffer);
uint8_t uint64t_to_bytes_array(uint64_t param, uint8_t* buf);

/**
 * @brief Transform a bytes array to a float
 * @param const unsigned byte array
 * @return float value
 */
float bytesarrayToFloat(const unsigned char * source);

/**
 * @brief Transform a bytes array to a unsigned int (32 bits)
 * @param *frame is the char array input
 * @return parsed output value (unsigned int 32 bits)
 */
unsigned int buffer_to_unsigned_int (unsigned char *frame);

/**
 * @brief Transform a unsigned int (32bits) to a byte array
 * @param input unsigned int input value
 * @param *frame byte array output
 * @return the input value
 */
unsigned int unsigned_int_to_byte_array (unsigned int input, unsigned char *frame);


void bytes_array_to_uint64t_by_ref(uint8_t* buffer, uint64_t * dest);

uint16_t swap_uint16(uint16_t val);
int16_t swap_int16(int16_t val);
uint32_t swap_uint32(uint32_t val);
int32_t swap_int32(int32_t val);
int64_t swap_int64(int64_t val);
uint64_t swap_uint64(uint64_t val);



#endif /* UTILS_DATA_OP_OP_H_ */
