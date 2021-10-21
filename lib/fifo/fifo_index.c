#include "fifo.h"

uint32_t volatile putI; // put next
uint32_t volatile getI; // get next

unsigned char fifo[FIFOSIZE];

void fifo_init_index(void){
	// make atomic if it aims to be used in an ISR
	putI = getI = 0;
}

int fifo_index_put(unsigned char data){
	if((putI - getI) & ~(FIFOSIZE-1)){
		return -1;
	}

	fifo[putI & (FIFOSIZE - 1)] = data;
	putI++;

	return 1;
}

int fifo_index_get(unsigned char * pData){
	if(putI == getI){
		return -1;
	}

	*pData = fifo[getI &(FIFOSIZE-1)];
	getI ++;

	return 0;
}


uint32_t fifo_index_size(void){
	return((uint32_t)(putI - getI));
}

