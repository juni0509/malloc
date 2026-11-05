#pragma once
#ifndef MEMORY_H
#define MEMORY_H

#include "types.h"

int  readInt(const Heap* h, int pos);
void writeInt(Heap* h, int pos, int value);

int  getSize(const Heap* h, int blk);
void setSize(Heap* h, int blk, int size);
int  getNext(const Heap* h, int blk);
void setNext(Heap* h, int blk, int next);

int  inFreeList(const Heap* h, int blk);  


int isBlockStart(const Heap* h, int pos);
#endif