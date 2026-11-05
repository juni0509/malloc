#pragma once
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

int  readInt(const Heap* heap, int position);
void writeInt(Heap* heap, int position, int value);

int  getSize(const Heap* heap, int block);
void setSize(Heap* heap, int block, int size);
int  getNext(const Heap* heap, int block);
void setNext(Heap* heap, int block, int next);

int  inFreeList(const Heap* heap, int block);  


int isBlockStart(const Heap* heap, int position);
#endif