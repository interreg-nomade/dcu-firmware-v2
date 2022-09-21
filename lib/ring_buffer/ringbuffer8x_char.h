/*
 * ringbuffer8x_char.h
 *
 *  Created on: 29 april 2022
 *  Adapted by: Sarah Goossens
 */

#ifndef RING_BUFFER_RINGBUFFER8X_CHAR_H_
#define RING_BUFFER_RINGBUFFER8X_CHAR_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE8X 4096 //8192  // size of ring buffer. Only RING_BUFFER_SIZE-1 items can be contained in buffer, must be a power of two.

#if (RING_BUFFER_SIZE8X & (RING_BUFFER_SIZE8X - 1)) != 0
#error "RING_BUFFER_SIZE must be a power of two"
#endif

typedef uint32_t ring_8xbuffer_size_t; // Type used to hold size and indices of buffer.

#define RING_BUFFER_MASK8X (RING_BUFFER_SIZE8X-1) // Used as modulo operator
//as <tt> a % b = (a & (b − 1)) </tt> where \c a is a positive index in the buffer and \c b is the (power of two) size of the buffer.

typedef struct ring_8xbuffer_t ring_8xbuffer_t;

struct ring_8xbuffer_t { //Structure which holds 8x ring buffer
  char buffer[RING_BUFFER_SIZE8X]; // Buffer memory
  ring_8xbuffer_size_t tail_index; // tail index
  ring_8xbuffer_size_t head_index; // head index
};

void ring_8xbuffer_init(ring_8xbuffer_t *buffer); // initializes buffer, can also be used to empty/reset buffer
void ring_8xbuffer_queue(ring_8xbuffer_t *buffer, char data); // adds byte to ring buffer
void ring_8xbuffer_queue_arr(ring_8xbuffer_t *buffer, const char *data, ring_8xbuffer_size_t size); // adds an array of bytes to ring buffer
uint8_t ring_8xbuffer_dequeue(ring_8xbuffer_t *buffer, char *data); // returns oldest byte in ring buffer. Returns 1 if data was returned, 0 otherwise.
uint8_t ring_8xbuffer_dequeue_arr(ring_8xbuffer_t *buffer, char *data, ring_8xbuffer_size_t len); // returns len oldest byte in ring buffer. Returns # of bytes returned.
uint8_t ring_8xbuffer_dequeue_arr_no_ret(ring_8xbuffer_t *buffer,  ring_8xbuffer_size_t len);
uint8_t ring_8xbuffer_peek(ring_8xbuffer_t *buffer, char *data, ring_8xbuffer_size_t index); // peeks element @index of ring buffer without removing it
uint8_t ring_8xbuffer_is_empty(ring_8xbuffer_t *buffer); // returns 1 when ring buffer is empty, 0 otherwise
uint8_t ring_8xbuffer_is_full(ring_8xbuffer_t *buffer); // returns 1 when ring buffer is full, 0 otherwise
ring_8xbuffer_size_t ring_8xbuffer_num_items(ring_8xbuffer_t *buffer); // Returns number of items in ring buffer

#endif /* RING_BUFFER_RINGBUFFER8X_CHAR_H_ */
