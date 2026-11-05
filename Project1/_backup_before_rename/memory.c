#include <string.h>
#include "memory.h"

void writeInt(Heap* h, int pos, int value) {
	memcpy(&h->heapMem[pos], &value, sizeof(int));
}

int readInt(const Heap* h, int pos) {
	int value;
	memcpy(&value, &h->heapMem[pos], sizeof(int));
	return value;
}

int getSize(const Heap* h, int blk) { return readInt(h, blk); }
void setSize(Heap* h, int blk, int size) { writeInt(h, blk, size);}
int getNext(const Heap* h, int blk) { return readInt(h, blk + 4); }
void setNext(Heap* h, int blk, int next) { writeInt(h, blk + 4, next); }

int inFreeList(const Heap* h, int blk) {
	int cur;
	for (cur = h->freeHead; cur != NIL; cur = getNext(h, cur)) {
		if(cur ==blk) return 1;
	}
	return 0;
}
// 0번부터 block을 하나씩 건너뛰며 pos와 만나는지 본다
int isBlockStart(const Heap* h, int pos) {
	int blk = 0;
	while (blk < h->heapSize) {
		int size = getSize(h, blk);
		if (blk == pos) return 1;
		if (size < 0 || blk + HEADER_SIZE + size > h->heapSize) return 0;
		blk += HEADER_SIZE + size;
	}
	return 0;
}