/*
 * storage_ring_buffer.h
 *
 *  Created on: Jan 30, 2020
 *      Author: aclem
 */

#ifndef STOMEAS_LIB_STORAGE_RING_BUFFER_H_
#define STOMEAS_LIB_STORAGE_RING_BUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

/**
 * The size of a ring buffer.
 * Due to the design only <tt> RING_BUFFER_SIZE-1 </tt> items
 * can be contained in the buffer.
 * The buffer size must be a power of two.
*/
#define MES_RING_BUFFER_SIZE  64
#define MES_RING_BUFFER_BLOCK_SIZE 1024

#if (RING_BUFFER_SIZE & (RING_BUFFER_SIZE - 1)) != 0
#error "RING_BUFFER_SIZE must be a power of two"
#endif

/**
 * The type which is used to hold the size
 * and the indicies of the buffer.
 * Must be able to fit \c RING_BUFFER_SIZE .
 */
typedef uint32_t mes_ring_buffer_size_t;

/**
 * Used as a modulo operator
 * as <tt> a % b = (a & (b − 1)) </tt>
 * where \c a is a positive index in the buffer and
 * \c b is the (power of two) size of the buffer.
 */
#define MES_RING_BUFFER_MASK (MES_RING_BUFFER_SIZE-1)

/**
 * Simplifies the use of <tt>struct ring_buffer_t</tt>.
 */
typedef struct mes_ring_buffer_t mes_ring_buffer_t;

/* Structure which holds a measurement */
typedef struct {
	  char buffer[MES_RING_BUFFER_BLOCK_SIZE];
	  unsigned int  blockElems;
	  unsigned int cycleCounter;

} measurement_ring_block;

/**
 * Structure which holds a ring buffer.
 * The buffer contains a buffer array
 * as well as metadata for the ring buffer.
 */
struct mes_ring_buffer_t {
  /** Buffer memory. */
  measurement_ring_block buffer[MES_RING_BUFFER_SIZE];
  //int  blockElems[MES_RING_BUFFER_SIZE];
  /** Index of tail. */
  mes_ring_buffer_size_t tail_index;
  /** Index of head. */
  mes_ring_buffer_size_t head_index;
};

/**
 * Initializes the ring buffer pointed to by <em>buffer</em>.
 * This function can also be used to empty/reset the buffer.
 * @param buffer The ring buffer to initialize.
 */
void mes_ring_buffer_init(mes_ring_buffer_t *buffer);

/**
 * Adds a byte to a ring buffer.
 * @param buffer The buffer in which the data should be placed.
 * @param data The byte to place.
 */
void mes_ring_buffer_queue(mes_ring_buffer_t *buffer, measurement_ring_block data);

/**
 * Returns the oldest byte in a ring buffer.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t mes_ring_buffer_dequeue(mes_ring_buffer_t *buffer, measurement_ring_block *data);

/**
 * Peeks a ring buffer, i.e. returns an element without removing it.
 * @param buffer The buffer from which the data should be returned.
 * @param data A pointer to the location at which the data should be placed.
 * @param index The index to peek.
 * @return 1 if data was returned; 0 otherwise.
 */
uint8_t mes_ring_buffer_peek(mes_ring_buffer_t *buffer, measurement_ring_block *data, mes_ring_buffer_size_t index);


/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
uint8_t mes_ring_buffer_is_empty(mes_ring_buffer_t *buffer);
/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
uint8_t mes_ring_buffer_is_full(mes_ring_buffer_t *buffer);

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
mes_ring_buffer_size_t mes_ring_buffer_num_items(mes_ring_buffer_t *buffer);

uint8_t mes_ring_buffer_dequeue_arr_no_ret(mes_ring_buffer_t *buffer,  mes_ring_buffer_size_t len);

#endif /* STOMEAS_LIB_STORAGE_RING_BUFFER_H_ */
