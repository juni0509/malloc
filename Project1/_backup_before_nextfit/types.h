#ifndef TYPES_H
#define TYPES_H

#define HEADER_SIZE 8  // size(4) + next(4)
#define NIL (-1)       // free list의 끝
#define MAX_SIZE 4096  // heap 최대 크기

// 오류 코드. 0은 "오류 없음"
typedef enum {
    OK = 0,
    BAD_SIZE,
    NO_SPACE,
    BAD_ADDRESS,
    DOUBLE_FREE,
    NOT_INIT
} ErrorCode;

// 할당 전략
typedef enum { FIRST_FIT = 0, BEST_FIT, NEXT_FIT } FitStrategy;

// first/best-fit 비교용 통계
typedef struct {
    long searchSteps;  // 탐색하며 확인한 free block 수 (누적)
    int  allocOk;      // 할당 성공 횟수
    int  allocFail;    // 할당 실패 횟수
    int  freeOk;       // 반환 성공 횟수
    long wasted;       // 쪼개지 않고 통째로 줘서 남은 칸 수
} Stats;

// heap 상태 요약
typedef struct {
    int freeCount;    // free block 개수
    int totalFree;    // free block size 합계
    int largestFree;  // 가장 큰 free block의 size
} HeapInfo;

typedef struct {
    unsigned char heapMem[MAX_SIZE];
    int heapSize;
    int freeHead;
    int isReady;
    int lastError;
    int strategy;    // FitStrategy
    int nextFitPos;  // next-fit이 다음에 탐색을 시작할 free block 위치
    Stats stats;
} Heap;

#endif