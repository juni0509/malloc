#include<string.h>
#include "manager.h"
#include "memory.h"


ErrorCode initHeap(Heap* h, int size) {
	if (size <HEADER_SIZE + 1 || size>MAX_SIZE) {
		return BAD_SIZE;
	}


	h->heapSize = size;
	h->freeHead = 0;
	h->isReady = 1;
	h->lastError = OK;
	h->nextFitPots= 0;
	memset(&h->stats, 0, sizeof(Stats));


	setSize(h, 0, size - HEADER_SIZE);
	setNext(h, 0, NIL);

	return OK;
}
