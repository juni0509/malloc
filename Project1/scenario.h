#ifndef SCENARIO_H
#define SCENARIO_H

#include "types.h"

int         caseCount(void);
const char* caseTitle(int id);
int         caseHeapSize(int id);
void        runCase(Heap* heap, int id, int verbose);
void        compareFirstBest(int id);

#endif