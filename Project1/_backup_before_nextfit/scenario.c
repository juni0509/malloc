#include <stdio.h>
#include "scenario.h"
#include "manager.h"
#include "print.h"

/* ---------- 도우미: 할당/반환하고, verbose면 결과를 출력한다 ---------- */
static int doAlloc(Heap* heap, const char* name, int requestSize, int verbose) {
    int addr = myalloc(heap, requestSize);
    if (verbose) {
        printf("%s = myalloc(%d) -> %d", name, requestSize, addr);
        if (addr < 0) printf("  (실패: %s)", errorText(heap->lastError));
        printf("\n    ");
        printFreeList(heap);
    }
    return addr;
}

static void doFree(Heap* heap, const char* name, int addr, int verbose) {
    ErrorCode result = myfree(heap, addr);
    if (verbose) {
        printf("myfree(%s) -> %s\n    ", name, errorText(result));
        printFreeList(heap);
    }
}

/* ---------- 시나리오들 ---------- */

/* 0: 계획서 예시 */
static void runExample(Heap* heap, int verbose) {
    int addrA, addrB, addrC;
    addrA = doAlloc(heap, "a", 10, verbose);
    addrB = doAlloc(heap, "b", 20, verbose);
    addrC = doAlloc(heap, "c", 10, verbose);
    doFree(heap, "a", addrA, verbose);
    addrA = doAlloc(heap, "a", 30, verbose);
    doFree(heap, "c", addrC, verbose);
    doAlloc(heap, "d", 20, verbose);  /* 실패해야 함 */
    doFree(heap, "b", addrB, verbose);
}

/* 1: 경계값과 오류 */
static void runErrorCases(Heap* heap, int verbose) {
    int fullAddr, addrP, addrQ;
    doAlloc(heap, "x", 0, verbose);    /* BAD_SIZE */
    doAlloc(heap, "x", -5, verbose);   /* BAD_SIZE */
    doAlloc(heap, "x", 101, verbose);  /* BAD_SIZE (최대는 100) */
    fullAddr = doAlloc(heap, "full", 100, verbose);
    doAlloc(heap, "y", 1, verbose);  /* NO_SPACE */
    doFree(heap, "full", fullAddr, verbose);
    doFree(heap, "주소 9", 9, verbose);         /* BAD_ADDRESS (block 시작이 아님) */
    doFree(heap, "주소 3", 3, verbose);         /* BAD_ADDRESS (8보다 작음) */
    doFree(heap, "주소 500", 500, verbose);     /* BAD_ADDRESS (heap 밖) */
    doFree(heap, "주소 8", 8, verbose);         /* DOUBLE_FREE */
    addrP = doAlloc(heap, "p", 91, verbose);  /* 남는 9 -> 쪼갬 */
    doFree(heap, "p", addrP, verbose);
    addrQ = doAlloc(heap, "q", 92, verbose);  /* 남는 8 -> 통째로 */
    doFree(heap, "q", addrQ, verbose);
}

/* 2: 병합 4가지 */
static void runMergeCases(Heap* heap, int verbose) {
    int addrA, addrB, addrC, addrD, addrE;
    addrA = doAlloc(heap, "a", 10, verbose);
    addrB = doAlloc(heap, "b", 10, verbose);
    addrC = doAlloc(heap, "c", 10, verbose);
    addrD = doAlloc(heap, "d", 10, verbose);
    addrE = doAlloc(heap, "e", 10, verbose);
    doFree(heap, "b", addrB, verbose);  /* 이웃 없음 */
    doFree(heap, "c", addrC, verbose);  /* 앞쪽만 */
    doFree(heap, "e", addrE, verbose);  /* 뒤쪽만 */
    doFree(heap, "d", addrD, verbose);  /* 양쪽 */
    doFree(heap, "a", addrA, verbose);  /* 전부 복원 */
}

/* 3: 전략마다 결과가 달라지는 시나리오 */
static void runFitCases(Heap* heap, int verbose) {
    int addrA, addrB, addrC;
    addrA = doAlloc(heap, "a", 50, verbose);
    doAlloc(heap, "s1", 10, verbose);  /* 사이에 낀 칸막이 */
    addrB = doAlloc(heap, "b", 20, verbose);
    doAlloc(heap, "s2", 10, verbose);
    addrC = doAlloc(heap, "c", 30, verbose);
    doAlloc(heap, "s3", 10, verbose);
    doAlloc(heap, "d", 100, verbose);
    doFree(heap, "a", addrA, verbose);  /* 구멍 50, 20, 30이 생김 */
    doFree(heap, "b", addrB, verbose);
    doFree(heap, "c", addrC, verbose);
    doAlloc(heap, "e", 15, verbose);
    doAlloc(heap, "f", 25, verbose);
    doAlloc(heap, "g", 18, verbose);
    doAlloc(heap, "h", 40, verbose);
    doAlloc(heap, "i", 10, verbose);
}

/* ---------- 목록 ---------- */
int caseCount(void) { return 4; }

const char* caseTitle(int id) {
    switch (id) {
    case 0:  return "S1 계획서 예시";
    case 1:  return "E1 경계값과 오류";
    case 2:  return "E2 병합 4가지";
    default: return "E3 first/best 차이";
    }
}

int caseHeapSize(int id) {
    switch (id) {
    case 0:  return 108;
    case 1:  return 108;
    case 2:  return 200;
    default: return 500;
    }
}

/* heap을 초기화하고 시나리오를 실행한다 (strategy는 그대로 유지) */
void runCase(Heap* heap, int id, int verbose) {
    initHeap(heap, caseHeapSize(id));
    switch (id) {
    case 0:  runExample(heap, verbose); break;
    case 1:  runErrorCases(heap, verbose); break;
    case 2:  runMergeCases(heap, verbose); break;
    default: runFitCases(heap, verbose); break;
    }
}

/* ---------- first, best-fit을 같은 시나리오로 돌려서 비교 (next-fit도 같이 나옴) ---------- */
void compareFirstBest(int id) {
    const char* names[] = { "first-fit", "best-fit", "next-fit" };
    int strategyId;

    printf("== %s (heap %d) ==\n", caseTitle(id), caseHeapSize(id));
    printf("%-10s %4s %5s %6s %6s %8s %6s\n",
        "strategy", "ok", "fail", "steps", "free#", "largest", "waste");

    for (strategyId = FIRST_FIT; strategyId <= NEXT_FIT; strategyId++) {
        Heap heap = { 0 };
        HeapInfo info;

        heap.strategy = strategyId;
        runCase(&heap, id, 0);  /* 출력 없이 실행 */
        info = getHeapInfo(&heap);
        printf("%-10s %4d %5d %6ld %6d %8d %6ld\n", names[strategyId],
            heap.stats.allocOk, heap.stats.allocFail, heap.stats.searchSteps,
            info.freeCount, info.largestFree, heap.stats.wasted);
    }
}