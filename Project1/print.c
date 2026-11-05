#include <stdio.h>
#include "print.h"
#include "memory.h"

#define MAP_WIDTH 50  /* 한 줄 막대의 칸 수 */

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
    default:          return "UNKNOWN";  // 위 어느 것도 아닐 때
    }
}

/* ---------- printFreeList ----------
 * 하는 일: free list를 계획서 모양으로 한 줄에 출력한다.
 * 예: HEAD -> [0, size 10] -> [64, size 36] -> END
 * 빈 block이 하나도 없으면: HEAD -> END
 */
void printFreeList(const Heap* heap) {
    int current;

    if (!heap->isReady) {  // 초기화 전이면 안내만 하고 끝
        printf("(heap이 아직 초기화되지 않았습니다)\n");
        return;
    }
    printf("HEAD");
    // inFreeList와 같은 방식으로 리스트를 따라가며 하나씩 출력
    for (current = heap->freeHead; current != NIL; current = getNext(heap, current)) {
        printf(" -> [%d, size %d]", current, getSize(heap, current));
    }
    printf(" -> END\n");
}

/* ---------- isFreeAt ----------
 * 하는 일: heap의 position번 칸이 "빈 block" 안에 있으면 1, 사용 중이면 0.
 * 원리: printHeapMap과 같이 block을 앞에서부터 건너뛰며 position가 속한 block을 찾는다.
 */
static int isFreeAt(const Heap* heap, int position) {
    int block = 0;

    while (block < heap->heapSize) {
        int size = getSize(heap, block);
        int end;

        if (size < 0 || block + HEADER_SIZE + size > heap->heapSize) return 0;
        end = block + HEADER_SIZE + size;
        if (position < end) return inFreeList(heap, block);
        block = end;
    }
    return 0;
}

/* ---------- printHeapMap ----------
 * 하는 일: heap 전체를 한 줄 막대(# 사용 중, . 빈 칸)와
 *          block별 표(시작, size, 상태)로 출력한다.
 * 막대: heap을 MAP_WIDTH칸으로 나눠서, 각 칸이 어느 block에 속하는지 본다.
 * 표:   "다음 block의 시작 = 지금 시작 + header(8) + size"로 건너뛰며 한 줄씩 출력.
 *       (사용 중인 block도 header에 size가 남아 있어서 건너뛸 수 있다)
 */
void printHeapMap(const Heap* heap) {
    int block = 0;  // 지금 보는 block의 시작
    int i;

    if (!heap->isReady) {
        printf("(heap이 아직 초기화되지 않았습니다)\n");
        return;
    }

    // 1) 한 줄 막대
    printf("  heap |");
    for (i = 0; i < MAP_WIDTH; i++) {
        printf("%c", isFreeAt(heap, i * heap->heapSize / MAP_WIDTH) ? '.' : '#');
    }
    printf("|  (# 사용 중, . 빈 칸)\n");

    // 2) block별 표
    printf("  ------------------------------\n");
    printf("  %5s  %5s  %s\n", "시작", "size", "상태");
    while (block < heap->heapSize) {
        int size = getSize(heap, block);

        // 안전장치: header가 망가져서 이상한 size면 무한 루프 방지를 위해 멈춘다
        if (size < 0 || block + HEADER_SIZE + size > heap->heapSize) {
            printf("  (%d번에서 header가 이상합니다)\n", block);
            break;
        }
        // %5d는 "5칸 폭으로 맞춰서 출력"이라 줄이 가지런해진다
        printf("  %5d  %5d  %s\n", block, size,
            inFreeList(heap, block) ? "빈 상태" : "사용 중");

        block += HEADER_SIZE + size;  // 다음 block의 시작으로 이동
    }
}
