#include <stdio.h>
#include "scenario.h"
#include "manager.h"
#include "print.h"

/* ---------- 도우미: 할당/반환하고, verbose면 결과를 출력한다 ---------- */
static int doAlloc(Heap* h, const char* name, int n, int verbose) {
    int addr = myalloc(h, n);
    if (verbose) {
        printf("%s = myalloc(%d) -> %d", name, n, addr);
        if (addr < 0) printf("  (실패: %s)", errorText(h->lastError));
        printf("\n    ");
        printFreeList(h);
    }
    return addr;
}

static void doFree(Heap* h, const char* name, int addr, int verbose) {
    ErrorCode e = myfree(h, addr);
    if (verbose) {
        printf("myfree(%s) -> %s\n    ", name, errorText(e));
        printFreeList(h);
    }
}

/* ---------- 시나리오들 ---------- */

/* 0: 계획서 예시 */
static void scenarioS1(Heap* h, int v) {
    int a, b, c;
    a = doAlloc(h, "a", 10, v);
    b = doAlloc(h, "b", 20, v);
    c = doAlloc(h, "c", 10, v);
    doFree(h, "a", a, v);
    a = doAlloc(h, "a", 30, v);
    doFree(h, "c", c, v);
    doAlloc(h, "d", 20, v);          /* 실패해야 함 */
    doFree(h, "b", b, v);
}

/* 1: 경계값과 오류 */
static void scenarioE1(Heap* h, int v) {
    int full, p, q;
    doAlloc(h, "x", 0, v);           /* BAD_SIZE */
    doAlloc(h, "x", -5, v);          /* BAD_SIZE */
    doAlloc(h, "x", 101, v);         /* BAD_SIZE (최대는 100) */
    full = doAlloc(h, "full", 100, v);
    doAlloc(h, "y", 1, v);           /* NO_SPACE */
    doFree(h, "full", full, v);
    doFree(h, "주소 9", 9, v);       /* BAD_ADDRESS (block 시작이 아님) */
    doFree(h, "주소 3", 3, v);       /* BAD_ADDRESS (8보다 작음) */
    doFree(h, "주소 500", 500, v);   /* BAD_ADDRESS (heap 밖) */
    doFree(h, "주소 8", 8, v);       /* DOUBLE_FREE */
    p = doAlloc(h, "p", 91, v);      /* 남는 9 -> 쪼갬 */
    doFree(h, "p", p, v);
    q = doAlloc(h, "q", 92, v);      /* 남는 8 -> 통째로 */
    doFree(h, "q", q, v);
}

/* 2: 병합 4가지 */
static void scenarioE2(Heap* h, int v) {
    int a, b, c, d, e;
    a = doAlloc(h, "a", 10, v);
    b = doAlloc(h, "b", 10, v);
    c = doAlloc(h, "c", 10, v);
    d = doAlloc(h, "d", 10, v);
    e = doAlloc(h, "e", 10, v);
    doFree(h, "b", b, v);            /* 이웃 없음 */
    doFree(h, "c", c, v);            /* 앞쪽만 */
    doFree(h, "e", e, v);            /* 뒤쪽만 */
    doFree(h, "d", d, v);            /* 양쪽 */
    doFree(h, "a", a, v);            /* 전부 복원 */
}

/* 3: 전략마다 결과가 달라지는 시나리오 */
static void scenarioE3(Heap* h, int v) {
    int a, b, c;
    a = doAlloc(h, "a", 50, v);
    doAlloc(h, "s1", 10, v);         /* 사이에 낀 칸막이 */
    b = doAlloc(h, "b", 20, v);
    doAlloc(h, "s2", 10, v);
    c = doAlloc(h, "c", 30, v);
    doAlloc(h, "s3", 10, v);
    doAlloc(h, "d", 100, v);
    doFree(h, "a", a, v);            /* 구멍 50, 20, 30이 생김 */
    doFree(h, "b", b, v);
    doFree(h, "c", c, v);
    doAlloc(h, "e", 15, v);
    doAlloc(h, "f", 25, v);
    doAlloc(h, "g", 18, v);
    doAlloc(h, "h", 40, v);
    doAlloc(h, "i", 10, v);
}

/* ---------- 목록 ---------- */
int scenarioCount(void) { return 4; }

const char* scenarioTitle(int id) {
    switch (id) {
    case 0:  return "S1 계획서 예시";
    case 1:  return "E1 경계값과 오류";
    case 2:  return "E2 병합 4가지";
    default: return "E3 전략별 차이";
    }
}

int scenarioHeapSize(int id) {
    switch (id) {
    case 0:  return 108;
    case 1:  return 108;
    case 2:  return 200;
    default: return 500;
    }
}

/* heap을 초기화하고 시나리오를 실행한다 (strategy는 그대로 유지) */
void runScenario(Heap* h, int id, int verbose) {
    initHeap(h, scenarioHeapSize(id));
    switch (id) {
    case 0:  scenarioS1(h, verbose); break;
    case 1:  scenarioE1(h, verbose); break;
    case 2:  scenarioE2(h, verbose); break;
    default: scenarioE3(h, verbose); break;
    }
}

/* ---------- 세 전략을 같은 시나리오로 돌려서 비교 ---------- */
void compareStrategies(int id) {
    const char* names[] = { "first-fit", "best-fit", "next-fit" };
    int s;

    printf("== %s (heap %d) ==\n", scenarioTitle(id), scenarioHeapSize(id));
    printf("%-10s %4s %5s %6s %6s %8s %7s %6s\n",
        "strategy", "ok", "fail", "steps", "free#", "largest", "frag%", "waste");

    for (s = FIRST_FIT; s <= NEXT_FIT; s++) {
        Heap h = { 0 };
        HeapInfo info;
        double frag = 0.0;

        h.strategy = s;
        runScenario(&h, id, 0);              /* 출력 없이 실행 */
        info = getHeapInfo(&h);
        if (info.totalFree > 0) {
            frag = 100.0 * (1.0 - (double)info.largestFree / info.totalFree);
        }
        printf("%-10s %4d %5d %6ld %6d %8d %7.1f %6ld\n", names[s],
            h.stats.allocOk, h.stats.allocFail, h.stats.searchSteps,
            info.freeCount, info.largestFree, frag, h.stats.wasted);
    }
}