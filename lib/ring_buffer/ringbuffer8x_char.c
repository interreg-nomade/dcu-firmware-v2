/*
 *  ringbuffer8x_char.c
 *
 *  Created on: 29 april 2022
 *  Adapted by: Sarah Goossens
 *
 */

#include "ringbuffer8x_char.h"
#include "../../app/app_tablet_com.h"
#include <string.h> // to declare strlen
#include "../../app/app_init.h" // to declare QueueHandle_t

#define Ringbuffer8x_DBG_PRINTF 0

extern char string[];
extern QueueHandle_t pPrintQueue;

void ring_8xbuffer_init(ring_8xbuffer_t *buffer)
{
  buffer->tail_index = 0;
  buffer->head_index = 0;
}

void ring_8xbuffer_queue(ring_8xbuffer_t *buffer, char data)
{
  if(ring_8xbuffer_is_full(buffer))
  { // overwrite oldest byte
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK8X);
  }
  buffer->buffer[buffer->head_index] = data; // place data in buffer
  buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK8X);
}

void ring_8xbuffer_queue_arr(ring_8xbuffer_t *buffer, const char *data, ring_8xbuffer_size_t size) {
  ring_8xbuffer_size_t i;
  for(i = 0; i < size; i++)
  { // add bytes one by one
    ring_8xbuffer_queue(buffer, data[i]);
  }
}

uint8_t ring_8xbuffer_dequeue(ring_8xbuffer_t *buffer, char *data) {
  if(ring_8xbuffer_is_empty(buffer))
  { // no items in buffer
    return 0;
  }
  *data = buffer->buffer[buffer->tail_index];
  buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK8X);
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue] head_index = %ld, tail_index = %ld\n",buffer->head_index,buffer->tail_index);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return 1;
}

uint8_t ring_8xbuffer_dequeue_arr(ring_8xbuffer_t *buffer, char *data, ring_8xbuffer_size_t len)
{
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue_arr] head_index = %lu, tail_index = %lu, Number of elements in ringbuffer before dequeue = %lu\n",buffer->head_index,buffer->tail_index,(buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
  xQueueSend(pPrintQueue, string, 0);
#endif
  if(ring_8xbuffer_is_empty(buffer))
  { // no items in buffer
    return 0;
  }
  char *data_ptr = data;
  ring_8xbuffer_size_t cnt = 0;
  while((cnt < len) && ring_8xbuffer_dequeue(buffer, data_ptr)) {
    cnt++;
    data_ptr++;
  }
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue_arr] head_index = %lu, tail_index = %lu, Number of elements in ringbuffer after dequeue = %lu\n",buffer->head_index,buffer->tail_index,(buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
  xQueueSend(pPrintQueue, string, 0);
#endif
  return cnt;
}

uint8_t ring_8xbuffer_dequeue_arr_no_ret(ring_8xbuffer_t *buffer,  ring_8xbuffer_size_t len) {
  if(ring_8xbuffer_is_empty(buffer))
  { // no items
    return 0;
  }
  buffer->tail_index = ((buffer->tail_index + len) & RING_BUFFER_MASK8X); // Dequeue array, by moving tail
  return 1;
}


uint8_t ring_8xbuffer_peek(ring_8xbuffer_t *buffer, char *data, ring_8xbuffer_size_t index) {
  if(index >= ring_8xbuffer_num_items(buffer))
  { // no items at index
    return 0;
  }
  ring_8xbuffer_size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK8X); // add index to pointer
  *data = buffer->buffer[data_index];
  return 1;
}

uint8_t ring_8xbuffer_is_empty(ring_8xbuffer_t *buffer)
{
  return (buffer->head_index == buffer->tail_index);
}

uint8_t ring_8xbuffer_is_full(ring_8xbuffer_t *buffer)
{
  return (((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK8X) == RING_BUFFER_MASK8X);
}

ring_8xbuffer_size_t ring_8xbuffer_num_items(ring_8xbuffer_t *buffer)
{
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK8X);
}
