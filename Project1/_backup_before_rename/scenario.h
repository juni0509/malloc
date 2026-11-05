#ifndef SCENARIO_H
#define SCENARIO_H

#include "types.h"

int         scenarioCount(void);
const char* scenarioTitle(int id);
int         scenarioHeapSize(int id);
void        runScenario(Heap* h, int id, int verbose);
void        compareStrategies(int id);

#endif