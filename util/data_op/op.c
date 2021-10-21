#include "op.h"

/* Converts a flow to a byte array
 * LSB
 * e.g.: 1.23 is represented in memory as 0x3f9d70a4
 * this outputs
 * dest[0] = 0xa4
 * dest[1] = 0x70
 * dest[2] = 0x9d
 * dest[3] = 0x3f
 */
int float_to_byte_array_lsb(float f, char * dest)
{
    if (dest == 0)
    {
        return 0;
    }

    float lf = f;
    unsigned char *p = (unsigned char*) &lf;

    *(dest++) = *(p++);
    *(dest++) = *(p++);
    *(dest++) = *(p++);
    *(dest)   = *(p);

    return 4;
}

int float_to_byte_array_msb(float f, char * dest)
{
    if (dest == 0)
    {
        return 0;
    }

    float lf = f;
    unsigned char *p = (unsigned char*) &lf;
    p += 3; /* Offset */
    *(dest++) = *(p--);
    *(dest++) = *(p--);
    *(dest++) = *(p--);
    *(dest)   = *(p);

    return 4;
}

int	short_to_byte_array_msb(short s, char * dest)
{
	if (dest == 0)
	{
		return 0;
	}

	*(dest++) 	= (s >> 8);
	* dest	  	= (s & 0x00ff);

	return 2;
}


int	short_to_byte_array_lsb(short s, char * dest)
{
	if (dest == 0)
	{
		return 0;
	}

	*(dest++) 	= (unsigned char) (s & 0x00ff);
	* dest	  	= (unsigned char) (s >> 8);

	return 2;
}


unsigned int rightshifty32_unsigned_integer_msb (unsigned int data, unsigned char *frame)
{
	unsigned int i = 0;

	/* Better to use shift => less computation time */
		frame[i++] = (data >> 24) & 0xFF;
		frame[i++] = (data >> 16) & 0xFF;
		frame[i++] = (data >> 8) & 0xFF;
		frame[i++] = data & 0xFF;

		return i;
}

unsigned int rightshifty32_signed_integer (int data, unsigned char *frame)
{
	unsigned int i = 0;

	/* Better to use shift => less computation time */

	frame[i++] = (data >> 24) & 0xFF;
	frame[i++] = (data >> 16) & 0xFF;
	frame[i++] = (data >> 8) & 0xFF;
	frame[i++] = data & 0xFF;

	return i;
}

uint64_t bytes_array_to_uint64t(uint8_t* buffer)
{
	if (!buffer)
	{
		return 0;
	}

	union {
		uint64_t parsed;
		uint8_t raw[8];

	} formater;

	for (uint32_t i = 0; i<8; i++)
	{
		formater.raw[7-i] = buffer[i];
	}

	return formater.parsed;
}

uint8_t uint64t_to_bytes_array(uint64_t param, uint8_t* buf)
{
	if (!buf)
	{
		return 0;
	}
	union {
			uint64_t parsed;
			uint8_t raw[8];
	} formater;

	formater.parsed = param;

	for (uint32_t i = 0; i<8; i++)
	{
		buf[i] = formater.raw[7-i];
	}
	return 1;
}

float bytesarrayToFloat(const unsigned char * source)
{
	const n_elem = 4;
	float output;

	for(unsigned int i = 0; i < n_elem; i++)
	{
		*((unsigned char *) (&output) + (3 - i)) = source[i];
		*((unsigned char *) (&output) + (3 - i)) = source[i];
		*((unsigned char *) (&output) + (3 - i)) = source[i];
		*((unsigned char *) (&output) + (3 - i)) = source[i];
	}

	return output;
}

unsigned int buffer_to_unsigned_int (unsigned char *frame)
{
	unsigned int i = 0;
	i = (frame[0] << 24) | (frame[1] << 16) | (frame[2] << 8) | (frame[3]);
	return (i);
}

unsigned int unsigned_int_to_byte_array (unsigned int input, unsigned char *frame)
{
	frame[0] = input >> 24;
	frame[1] = input >> 16;
	frame[2] = input >> 8;
	frame[3] = input & 0xff;

	return input;
}


void bytes_array_to_uint64t_by_ref(uint8_t* buffer, uint64_t * dest)
{
	if (!buffer)
	{
		return;
	}

	union {
		uint64_t parsed;
		uint8_t raw[8];

	} formater;

	for (uint32_t i = 0; i<8; i++)
	{
		formater.raw[7-i] = buffer[i];
	}

	*dest = formater.parsed;

	return;
}


//! Byte swap unsigned short
uint16_t swap_uint16( uint16_t val )
{
    return (val << 8) | (val >> 8 );
}

//! Byte swap short
int16_t swap_int16( int16_t val )
{
    return (val << 8) | ((val >> 8) & 0xFF);
}

//! Byte swap unsigned int
uint32_t swap_uint32( uint32_t val )
{
    val = ((val << 8) & 0xFF00FF00 ) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | (val >> 16);
}

//! Byte swap int
int32_t swap_int32( int32_t val )
{
    val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF );
    return (val << 16) | ((val >> 16) & 0xFFFF);
}

int64_t swap_int64( int64_t val )
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
}

uint64_t swap_uint64( uint64_t val )
{
    val = ((val << 8) & 0xFF00FF00FF00FF00ULL ) | ((val >> 8) & 0x00FF00FF00FF00FFULL );
    val = ((val << 16) & 0xFFFF0000FFFF0000ULL ) | ((val >> 16) & 0x0000FFFF0000FFFFULL );
    return (val << 32) | (val >> 32);
}

