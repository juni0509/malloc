#pragma once
#ifndef PRINT_H
#define PRINT_H

#include "types.h"

void printFreeList(const Heap* heap);  // free list를 한 줄로 출력
void printHeapMap(const Heap* heap);   // heap 전체를 block별로 출력
const char* errorText(int code);       // 오류 코드 → 글자

#endif