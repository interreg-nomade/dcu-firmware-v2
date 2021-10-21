#ifndef DATA_FIFO_H_
#define DATA_FIFO_H_

#include <stdint.h>

#define FIFOSIZE		2048

/* For fifo used by index */
void fifo_init_index(void);
int fifo_index_put(unsigned char data);
int fifo_index_get(unsigned char * pData);
uint32_t fifo_index_size(void);

/* For fifo used by pointer */
void fifo_init_pt(void);
int fifo_pt_put(unsigned char data);
int fifo_pt_get(unsigned char * pData);
uint32_t fifo_pt_size(void);

#endif /* DATA_FIFO_H_ */
