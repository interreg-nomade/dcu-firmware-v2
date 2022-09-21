/*
 *  ringbuffer4x_char.h
 *
 *  Created on: 29 april 2022
 *  Adapted by: Sarah Goossens
 *
 *  Prototypes and structures for the ring buffer 4x size module.
 */

#ifndef RING_BUFFER_RINGBUFFER4X_CHAR_H_
#define RING_BUFFER_RINGBUFFER4X_CHAR_H_

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#define RING_BUFFER_SIZE4X 4096  // size of ring buffer. Only RING_BUFFER_SIZE-1 items can be contained in buffer, must be a power of two.

#if (RING_BUFFER_SIZE4X & (RING_BUFFER_SIZE4X - 1)) != 0
#error "RING_BUFFER_SIZE must be a power of two"
#endif

typedef uint32_t ring_4xbuffer_size_t; // Type used to hold size and indices of buffer.

#define RING_BUFFER_MASK4X (RING_BUFFER_SIZE4X-1) // Used as modulo operator
//as <tt> a % b = (a & (b − 1)) </tt> where \c a is a positive index in the buffer and \c b is the (power of two) size of the buffer.

typedef struct ring_4xbuffer_t ring_4xbuffer_t;

struct ring_4xbuffer_t { //Structure which holds 4x ring buffer
  char buffer[RING_BUFFER_SIZE4X]; // Buffer memory
  ring_4xbuffer_size_t tail_index; // tail index
  ring_4xbuffer_size_t head_index; // head index
};

void ring_4xbuffer_init(ring_4xbuffer_t *buffer); // initializes buffer, can also be used to empty/reset buffer
void ring_4xbuffer_queue(ring_4xbuffer_t *buffer, char data); // adds byte to ring buffer
void ring_4xbuffer_queue_arr(ring_4xbuffer_t *buffer, const char *data, ring_4xbuffer_size_t size); // adds an array of bytes to ring buffer
uint8_t ring_4xbuffer_dequeue(ring_4xbuffer_t *buffer, char *data); // returns oldest byte in ring buffer. Returns 1 if data was returned, 0 otherwise.
uint8_t ring_4xbuffer_dequeue_arr(ring_4xbuffer_t *buffer, char *data, ring_4xbuffer_size_t len); // returns len oldest byte in ring buffer. Returns # of bytes returned.
uint8_t ring_4xbuffer_dequeue_arr_no_ret(ring_4xbuffer_t *buffer,  ring_4xbuffer_size_t len);
uint8_t ring_4xbuffer_peek(ring_4xbuffer_t *buffer, char *data, ring_4xbuffer_size_t index); // peeks element @index of ring buffer without removing it
uint8_t ring_4xbuffer_is_empty(ring_4xbuffer_t *buffer); // returns 1 when ring buffer is empty, 0 otherwise
uint8_t ring_4xbuffer_is_full(ring_4xbuffer_t *buffer); // returns 1 when ring buffer is full, 0 otherwise
ring_4xbuffer_size_t ring_4xbuffer_num_items(ring_4xbuffer_t *buffer); // Returns number of items in ring buffer

#endif /* RING_BUFFER_RINGBUFFER4X_CHAR_H_ */
