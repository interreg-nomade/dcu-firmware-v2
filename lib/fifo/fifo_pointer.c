#include "fifo.h"


unsigned char * pPut;
unsigned char * pGet;

unsigned char fifo[FIFOSIZE];


void fifo_init_pt(void){
	//make it atomic if used in an ISR
	pPut = pGet = &fifo[0];
}

int fifo_pt_put(unsigned char data){
	unsigned char volatile * nextpPut;
	nextpPut = pPut + 1;

	if(nextpPut == &fifo[FIFOSIZE]){
			nextpPut =  &fifo[0];
	}

	if(nextpPut == pGet){
		return -1; //fifo is full
	}
	else{
		*(pPut) = data;
		pPut = nextpPut;

		return 1;
	}
}

int fifo_pt_get(unsigned char * pData){
	if(pPut == pGet){
		return -1; //fifo is empty
	}

	*pData = *(pGet++);

	if(pGet == &fifo[FIFOSIZE]){
		pGet = &fifo[0];
	}

	return 1;
}

uint32_t fifo_pt_size(void){
	if(pPut < pGet){
	    return ((uint32_t)(pPut-pGet+(FIFOSIZE*sizeof(unsigned char)))/sizeof(unsigned char));
	  }
	  return ((uint32_t)(pPut-pGet)/sizeof(unsigned char));
}


