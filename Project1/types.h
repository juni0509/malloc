#ifndef TYPES_H
#define TYPES_H

#define HEADER_SIZE 8
#define NIL (-1)
#define MAX_SIZE 4096

typedef enum { FIRST_FIT = 0, BEST_FIT, NEXT_FIT }FitStrategy;

typedef struct {
	long searchSteps;
	int allockOk;
	int allocFail;
	int freeOk;
	long wasted;
}Stats;


typedef	struct {
	unsigned char heapMem[MAX_SIZE];
	int heapSize;
	int freeHead;
	int isReady;
	int lastError;
	int strategy;
	Stats stats;
}Heap;

#endif