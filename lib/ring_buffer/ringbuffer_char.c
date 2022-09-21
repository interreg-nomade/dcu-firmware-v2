/*
 *  ringbuffer_char.c
 *
 *  Created on: 29 april 2022
 *  Adapted by: Sarah Goossens
 *
 */

#include "ringbuffer_char.h"
#include "../../app/app_tablet_com.h"
#include <string.h> // to declare strlen
#include "../../app/app_init.h" // to declare QueueHandle_t

#define Ringbuffer_DBG_PRINTF 0

extern char string[];
extern QueueHandle_t pPrintQueue;

void ring_buffer_init(ring_buffer_t *buffer)
{
  buffer->tail_index = 0;
  buffer->head_index = 0;
}

void ring_buffer_queue(ring_buffer_t *buffer, char data)
{
  if(ring_buffer_is_full(buffer))
  { // overwrite oldest byte
    buffer->tail_index = ((buffer->tail_index + 1) & RING_BUFFER_MASK);
  }
  buffer->buffer[buffer->head_index] = data; // place data in buffer
  buffer->head_index = ((buffer->head_index + 1) & RING_BUFFER_MASK);
}

void ring_buffer_queue_arr(ring_buffer_t *buffer, const char *data, ring_buffer_size_t size) {
  ring_buffer_size_t i;
  for(i = 0; i < size; i++)
  { // add bytes one by one
    ring_buffer_queue(buffer, data[i]);
  }
}

uint8_t ring_buffer_dequeue(ring_buffer_t *buffer, char *data) {
  if(ring_buffer_is_empty(buffer))
  { // no items in buffer
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

uint8_t ring_buffer_dequeue_arr(ring_buffer_t *buffer, char *data, ring_buffer_size_t len)
{
#if Ringbuffer_DBG_PRINTF
  sprintf(string, "[Ringbuffer] [ring_buffer_dequeue_arr] head_index = %lu, tail_index = %lu, Number of elements in ringbuffer before dequeue = %lu\n",buffer->head_index,buffer->tail_index,(buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
  xQueueSend(pPrintQueue, string, 0);
#endif
  if(ring_buffer_is_empty(buffer))
  { // no items in buffer
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

uint8_t ring_buffer_dequeue_arr_no_ret(ring_buffer_t *buffer,  ring_buffer_size_t len) {
  if(ring_buffer_is_empty(buffer))
  { // no items
    return 0;
  }
  buffer->tail_index = ((buffer->tail_index + len) & RING_BUFFER_MASK); // Dequeue array, by moving tail
  return 1;
}


uint8_t ring_buffer_peek(ring_buffer_t *buffer, char *data, ring_buffer_size_t index) {
  if(index >= ring_buffer_num_items(buffer))
  { // no items at index
    return 0;
  }
  ring_buffer_size_t data_index = ((buffer->tail_index + index) & RING_BUFFER_MASK); // add index to pointer
  *data = buffer->buffer[data_index];
  return 1;
}

uint8_t ring_buffer_is_empty(ring_buffer_t *buffer)
{
  return (buffer->head_index == buffer->tail_index);
}

uint8_t ring_buffer_is_full(ring_buffer_t *buffer)
{
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK) == RING_BUFFER_MASK;
}

ring_buffer_size_t ring_buffer_num_items(ring_buffer_t *buffer)
{
  return ((buffer->head_index - buffer->tail_index) & RING_BUFFER_MASK);
}
