#ifndef MANAGER_H
#define MANAGER_H

#include "types.h"



ErrorCode initHeap(Heap* h, int size);
ErrorCode checkAllocRequest(const Heap* h, int n);
int findFirstFit(Heap* h, int n, int* prev);

int myalloc(Heap* h, int n);

// manager.h (#endif 위에)
ErrorCode myfree(Heap* h, int addr);
HeapInfo getHeapInfo(const Heap* h);
#endif

