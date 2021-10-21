#include "ringbuffer_char.h"
#include "../../app/app_tablet_com.h"
#include "usart.h"
#include <string.h> // om de functie strlen te hebben
#include "usart.h"  //to declare huart5
#include "app_init.h" // to declare QueueHandle_t

#define Ringbuffer_DBG_PRINTF 0

extern char string[];
extern QueueHandle_t pPrintQueue;

/**
 * @file
 * Implementation of ring buffer functions.
 */

void ring_buffer_init(ring_buffer_t *buffer) {
  buffer->tail_index = 0;
  buffer->head_index = 0;
}

void ring_buffer_queue(ring_buffer_t *buffer, char data) {
  /* Is buffer full? */
  if(ring_buffer_is_full(buffer)) {
    /* Is going to overwrite the oldest byte */
    /* Increase tail index */
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK);
  }

  /* Place data in buffer */
  buffer->buffer[buffer->head_index] = data;
  buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK);
//    if (buffer->head_index - buffer->tail_index >= 1) {
//	  sprintf(string, "buffer->head_index %d \n",buffer->head_index);
//  xQueueSend(pPrintQueue, string, 0);
//	  sprintf(string, "buffer->buffer 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", buffer->buffer[buffer->head_index-11], buffer->buffer[buffer->head_index-10], buffer->buffer[buffer->head_index-9], buffer->buffer[buffer->head_index-8], buffer->buffer[buffer->head_index-7], buffer->buffer[buffer->head_index-6], buffer->buffer[buffer->head_index-5], buffer->buffer[buffer->head_index-4], buffer->buffer[buffer->head_index-3], buffer->buffer[buffer->head_index-2],buffer->buffer[buffer->head_index-1],buffer->buffer[buffer->head_index]);
//  xQueueSend(pPrintQueue, string, 0);
//	}
}

void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size) {
  /* Add bytes; one by one */
  ring_buffer_size_t i;
  for(i = 0; i < size; i++) {
    ring_buffer_queue(buffer, data[i]);
  }
}

uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data) {
	if(ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK);
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue] head_index = %ld, tail_index = %ld\n",buffer->head_index,buffer->tail_index);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return 1;
}

uint8_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len) {
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue_arr] head_index = %lu, tail_index = %lu, Number of elements in ringbuffer before dequeue = %lu\n",buffer->head_index,buffer->tail_index,(buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
  xQueueSend(pPrintQueue, string, 0);
#endif
    if(ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }
  char *data_ptr = data;
  ring_buffer_size_t cnt = 0;
  while((cnt < len) && ring_buffer_dequeue(buffer, data_ptr)) {
    cnt++;
    data_ptr++;
  }
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue_arr] head_index = %lu, tail_index = %lu, Number of elements in ringbuffer after dequeue = %lu\n",buffer->head_index,buffer->tail_index,(buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return cnt;
}

uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index) {
  if(index >= ring_buffer_num_items(buffer)) {
    /* No items at index */
    return 0;
  }

  /* Add index to pointer */
  ring_buffer_size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK);
  *data = buffer->buffer[data_index];
  return 1;
}

/**
 * Returns whether a ring buffer is empty.
 * @param buffer The buffer for which it should be returned whether it is empty.
 * @return 1 if empty; 0 otherwise.
 */
uint8_t ring_buffer_is_empty(ring_buffer_t *buffer) {
  return (buffer->head_index == buffer->tail_index);
}

/**
 * Returns whether a ring buffer is full.
 * @param buffer The buffer for which it should be returned whether it is full.
 * @return 1 if full; 0 otherwise.
 */
uint8_t ring_buffer_is_full(ring_buffer_t *buffer) {
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK) == RING_BUFFER_MASK;
}

/**
 * Returns the number of items in a ring buffer.
 * @param buffer The buffer for which the number of items should be returned.
 * @return The number of items in the ring buffer.
 */
ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer) {
/*	my_printf("%d\n", (buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);*/ //TODO: remoev, test purpose.
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
}

/* Dequeue an array, no return (only moving the tail) */
uint8_t ring_buffer_dequeue_arr_no_ret(ring_buffer_t *buffer,  ring_buffer_size_t len) {
  if(ring_buffer_is_empty(buffer)) {
    /* No items */
    return 0;
  }

  buffer->tail_index = ((buffer->tail_index + len) & RING_BUFFER_MASK);
  return 1;
}

