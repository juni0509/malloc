#include <string.h>
#include "memory.h"

void writeInt(Heap* heap, int position, int value) {
	memcpy(&heap->heapMem[position], &value, sizeof(int));
}

int readInt(const Heap* heap, int position) {
	int value;
	memcpy(&value, &heap->heapMem[position], sizeof(int));
	return value;
}

int getSize(const Heap* heap, int block) { return readInt(heap, block); }
void setSize(Heap* heap, int block, int size) { writeInt(heap, block, size);}
int getNext(const Heap* heap, int block) { return readInt(heap, block + 4); }
void setNext(Heap* heap, int block, int next) { writeInt(heap, block + 4, next); }

int inFreeList(const Heap* heap, int block) {
	int current;
	for (current = heap->freeHead; current != NIL; current = getNext(heap, current)) {
		if(current ==block) return 1;
	}
	return 0;
}
// 0번부터 block을 하나씩 건너뛰며 position와 만나는지 본다
int isBlockStart(const Heap* heap, int position) {
	int block = 0;
	while (block < heap->heapSize) {
		int size = getSize(heap, block);
		if (block == position) return 1;
		if (size < 0 || block + HEADER_SIZE + size > heap->heapSize) return 0;
		block += HEADER_SIZE + size;
	}
	return 0;
}