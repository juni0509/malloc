#include <stdio.h>
#include "print.h"
#include "memory.h"

/* ---------- errorText ----------
 * 하는 일: 오류 코드(숫자)를 사람이 읽는 글자로 바꿔 준다.
 * 예: NO_SPACE(2) → "NO_SPACE"
 * switch는 값에 따라 갈래를 나누는 문장이고, return으로 바로 돌아간다.
 */
const char* errorText(int code) {
    switch (code) {
    case OK:          return "OK";
    case BAD_SIZE:    return "BAD_SIZE";
    case NO_SPACE:    return "NO_SPACE";
    case BAD_ADDRESS: return "BAD_ADDRESS";
    case DOUBLE_FREE: return "DOUBLE_FREE";
    case NOT_INIT:    return "NOT_INIT";
    default:          return "UNKNOWN";   // 위 어느 것도 아닐 때
    }
}

/* ---------- printFreeList ----------
 * 하는 일: free list를 계획서 모양으로 한 줄에 출력한다.
 * 예: HEAD -> [0, size 10] -> [64, size 36] -> END
 * 빈 block이 하나도 없으면: HEAD -> END
 */
void printFreeList(const Heap* h) {
    int cur;

    if (!h->isReady) {                 // 초기화 전이면 안내만 하고 끝
        printf("(heap이 아직 초기화되지 않았습니다)\n");
        return;
    }
    printf("HEAD");
    // inFreeList와 같은 방식으로 리스트를 따라가며 하나씩 출력
    for (cur = h->freeHead; cur != NIL; cur = getNext(h, cur)) {
        printf(" -> [%d, size %d]", cur, getSize(h, cur));
    }
    printf(" -> END\n");
}

/* ---------- printHeapMap ----------
 * 하는 일: heap 전체를 앞(0번)에서부터 block 단위로 한 줄씩 출력한다.
 *   각 줄: 시작 위치, size, 빈 상태/사용 중
 * 원리: block은 빈틈없이 이어져 있다. 그래서
 *       "다음 block의 시작 = 지금 시작 + header(8) + size"
 *       로 계속 건너뛰면 heap 끝까지 갈 수 있다.
 *       (사용 중인 block도 header에 size가 남아 있어서 건너뛸 수 있다)
 */
void printHeapMap(const Heap* h) {
    int blk = 0;                       // 지금 보는 block의 시작

    if (!h->isReady) {
        printf("(heap이 아직 초기화되지 않았습니다)\n");
        return;
    }
    while (blk < h->heapSize) {
        int size = getSize(h, blk);

        // 안전장치: header가 망가져서 이상한 size면 무한 루프 방지를 위해 멈춘다
        if (size < 0 || blk + HEADER_SIZE + size > h->heapSize) {
            printf("  (%d번에서 header가 이상합니다)\n", blk);
            break;
        }
        // %4d는 "4칸 폭으로 맞춰서 출력"이라 줄이 가지런해진다
        // (조건) ? A : B 는 조건이 참이면 A, 거짓이면 B
        printf("  시작 %4d  size %4d  %s\n", blk, size,
            inFreeList(h, blk) ? "빈 상태" : "사용 중");

        blk += HEADER_SIZE + size;     // 다음 block의 시작으로 이동
    }
}