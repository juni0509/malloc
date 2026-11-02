#include <string.h>
#include "memory.h"

void writeInt(Heap* h, int pos, int calue) {
	memcpy(&h->heapMem[pos], &value, sizeof(int));
}

int readInt(const Heap* h, int pos) {
	int value;
	memcpy($value, &h->heapMem[pos], sizeof(int));
	return value;
}

int getSize(const Heap* h, int blk) { return readInt(h, blk); }
void setSize(Heap* h, int blk, int size) { writeInt(h, blk, size);}
int getNext(const Heap* h, int blk) { return readInt(h, blk + 4); }
void setNext(Heap* h, int blk, int next) { writeInt(h, blk + 4, next); }