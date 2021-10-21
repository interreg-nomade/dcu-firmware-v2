#include "storage_ring_buffer.h"

/**
 * @file
 * Implementation of ring buffer functions.
 */

void mes_ring_buffer_init(mes_ring_buffer_t *buffer) {
  buffer->tail_index = 0;
  buffer->head_index = 0;
}

void mes_ring_buffer_queue(mes_ring_buffer_t *buffer, measurement_ring_block data) {
  /* Is buffer full? */
  if(mes_ring_buffer_is_full(buffer)) {
    /* Is going to overwrite the oldest byte */
    /* Increase tail index */
    buffer->tail_index = ((buffer->tail_index + 1) & MES_RING_BUFFER_MASK);
  }

  /* Place data in buffer */
  buffer->buffer[buffer->head_index] = data;
  buffer->head_index = ((buffer->head_index + 1) & MES_RING_BUFFER_MASK);
}

uint8_t mes_ring_buffer_dequeue(mes_ring_buffer_t *buffer, measurement_ring_block *data) {
  if(mes_ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & MES_RING_BUFFER_MASK);
  return 1;
}

uint8_t mes_ring_buffer_peek(mes_ring_buffer_t *buffer, measurement_ring_block *data, mes_ring_buffer_size_t index) {
  if(index >= mes_ring_buffer_num_items(buffer)) {
    /* No items at index */
    return 0;
  }

  /* Add index to pointer */
  mes_ring_buffer_size_t data_index = ((buffer->tail_index + index) & MES_RING_BUFFER_MASK);
  *data = buffer->buffer[data_index];
  return 1;
}

/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
uint8_t mes_ring_buffer_is_empty(mes_ring_buffer_t *buffer) {
  return (buffer->head_index == buffer->tail_index);
}

/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
uint8_t mes_ring_buffer_is_full(mes_ring_buffer_t *buffer) {
  return ((buffer->head_index - buffer->tail_index) & MES_RING_BUFFER_MASK) == MES_RING_BUFFER_MASK;
}

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
mes_ring_buffer_size_t mes_ring_buffer_num_items(mes_ring_buffer_t *buffer) {
/*	my_printf("%d\n", (buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);*/ //TODO: remoev, test purpose.
  return ((buffer->head_index - buffer->tail_index) & MES_RING_BUFFER_MASK);
}

/* Dequeue an array, no return (only moving the tail) */
uint8_t mes_ring_buffer_dequeue_arr_no_ret(mes_ring_buffer_t *buffer,  mes_ring_buffer_size_t len) {
  if(mes_ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  buffer->tail_index = ((buffer->tail_index + len) & MES_RING_BUFFER_MASK);
  return 1;
}
