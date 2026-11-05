#ifndef MANAGER_H
#define MANAGER_H

#include "types.h"



ErrorCode initHeap(Heap* heap, int size);
ErrorCode checkAllocRequest(const Heap* heap, int requestSize);
int findFirstFit(Heap* heap, int requestSize, int* previous);

int myalloc(Heap* heap, int requestSize);

// manager.h (#endif 위에)
ErrorCode myfree(Heap* heap, int addr);
HeapInfo getHeapInfo(const Heap* heap);
#endif

