#include<string.h>
#include "manager.h"
#include "memory.h"


ErrorCode initHeap(Heap* heap, int size) {
	if (size <HEADER_SIZE + 1 || size>MAX_SIZE) {
		return BAD_SIZE;
	}


	heap->heapSize = size;
	heap->freeHead = 0;
	heap->isReady = 1;
	heap->lastError = OK;
	memset(&heap->stats, 0, sizeof(Stats));


	setSize(heap, 0, size - HEADER_SIZE);
	setNext(heap, 0, NIL);

	return OK;
}
/* ---------- checkAllocRequest ----------
 * myalloc 요청이 올바른지 본다. 문제없으면 OK.
 * 초기화 여부를 먼저 보고, 그다음에 크기를 본다.
 */
ErrorCode checkAllocRequest(const Heap *heap, int requestSize) {
    if (!heap->isReady) return NOT_INIT;
    if (requestSize < 1 || requestSize > heap->heapSize - HEADER_SIZE) return BAD_SIZE;
    return OK;
}

/* ---------- findFirstFit ----------
 * free list를 앞에서부터 보며 size가 n 이상인 "첫 block"을 찾는다.
 * 돌려주는 값: 찾은 block의 시작 위치 (없으면 NIL)
 * *previous: 찾은 block 바로 앞 free block (맨 앞이면 NIL)
 */
int findFirstFit(Heap *heap, int requestSize, int *previous) {
    int before = NIL;
    int current = heap->freeHead;

    while (current != NIL) {
        heap->stats.searchSteps++;
        if (getSize(heap, current) >= requestSize) {
            *previous = before;
            return current;
        }
        before = current;
        current = getNext(heap, current);
    }
    *previous = NIL;
    return NIL;
}
/* ---------- myalloc ----------
 * 하는 일: n bytes를 할당하고, 사용자에게 줄 주소를 돌려준다. 실패하면 -1.
 * 실패 사유는 heap->lastError에 적어 둔다.
 */
 /* ---------- findBestFit ----------
  * 맞는 block(size >= n) 중에서 "가장 작은 것"을 고른다.
  * 크기가 같으면 리스트에서 먼저 나온 것을 고른다 (< 로 비교하니까).
  * first-fit과 달리 첫 번째에서 멈추지 않고 끝까지 본다.
  */
int findBestFit(Heap* heap, int requestSize, int* previous) {
    int before = NIL, current = heap->freeHead;
    int bestBlock = NIL, bestPrevious = NIL;  // 지금까지 찾은 최고 후보와 그 앞 block

    while (current != NIL) {
        heap->stats.searchSteps++;
        if (getSize(heap, current) >= requestSize) {
            if (bestBlock == NIL || getSize(heap, current) < getSize(heap, bestBlock)) {
                bestBlock = current;  // 더 작은 후보가 나왔다
                bestPrevious = before;
            }
        }
        before = current;
        current = getNext(heap, current);
    }
    *previous = bestPrevious;
    return bestBlock;
}

int myalloc(Heap* heap, int requestSize) {
    int previous, block, leftover, newBlock;
    ErrorCode checkResult = checkAllocRequest(heap, requestSize);

    /* 1. 요청이 올바른가? */
    if (checkResult != OK) {
        heap->lastError = checkResult;
        heap->stats.allocFail++;
        return -1;
    }

    /* 2. 맞는 free block을 찾는다 */
    if (heap->strategy == BEST_FIT) block = findBestFit(heap, requestSize, &previous);
    else                         block = findFirstFit(heap, requestSize, &previous);
    if (block == NIL) {
        heap->lastError = NO_SPACE;
        heap->stats.allocFail++;
        return -1;
    }

    /* 3. 쪼갤까, 통째로 줄까? */
    leftover = getSize(heap, block) - requestSize;
    if (leftover >= HEADER_SIZE + 1) {
        /* 쪼갠다 */
        newBlock = block + HEADER_SIZE + requestSize;
        setSize(heap, newBlock, leftover - HEADER_SIZE);
        setNext(heap, newBlock, getNext(heap, block));
        if (previous == NIL) heap->freeHead = newBlock;
        else             setNext(heap, previous, newBlock);
        setSize(heap, block, requestSize);
    }
    else {
        /* 통째로 준다 */
        if (previous == NIL) heap->freeHead = getNext(heap, block);
        else             setNext(heap, previous, getNext(heap, block));
        heap->stats.wasted += leftover;
    }

    /* 4. 마무리 */
    setNext(heap, block, NIL);  // 사용 중 block은 next = -1
    heap->lastError = OK;
    heap->stats.allocOk++;
    return block + HEADER_SIZE;  // 사용자 주소 = 시작 + 8
}
// manager.c (맨 아래에)

/* free list에서 block를 뺀다 */
static void removeFromList(Heap* heap, int block) {
    int previous = NIL;
    int current = heap->freeHead;

    while (current != NIL && current != block) {
        previous = current;
        current = getNext(heap, current);
    }
    if (current == NIL) return;  /* 목록에 없으면 할 일 없음 */

    if (previous == NIL) heap->freeHead = getNext(heap, current);
    else             setNext(heap, previous, getNext(heap, current));
}

/* 끝이 position와 딱 맞닿는 free block(앞쪽 이웃)을 찾는다. 없으면 NIL */
static int findFrontNeighbor(const Heap* heap, int position) {
    int current;
    for (current = heap->freeHead; current != NIL; current = getNext(heap, current)) {
        if (current + HEADER_SIZE + getSize(heap, current) == position) return current;
    }
    return NIL;
}

/* ---------- myfree ----------
 * addr(사용자 주소)의 block을 돌려받는다. 성공하면 OK, 아니면 오류 코드.
 */
ErrorCode myfree(Heap* heap, int addr) {
    int block = addr - HEADER_SIZE;
    int mergedStart, mergedSize, backNeighbor, frontNeighbor;

    /* 1. 주소 검사 (순서 중요) */
    if (!heap->isReady) { heap->lastError = NOT_INIT;    return NOT_INIT; }
    if (!isBlockStart(heap, block)) { heap->lastError = BAD_ADDRESS; return BAD_ADDRESS; }
    if (inFreeList(heap, block)) { heap->lastError = DOUBLE_FREE; return DOUBLE_FREE; }

    mergedStart = block;
    mergedSize = getSize(heap, block);

    /* 2. 뒤쪽 이웃: 내 block이 끝나는 자리에서 시작하는 free block */
    backNeighbor = mergedStart + HEADER_SIZE + mergedSize;
    if (inFreeList(heap, backNeighbor)) {
        mergedSize += HEADER_SIZE + getSize(heap, backNeighbor);
        removeFromList(heap, backNeighbor);
    }

    /* 3. 앞쪽 이웃: 끝이 내 시작과 맞닿는 free block */
    frontNeighbor = findFrontNeighbor(heap, mergedStart);
    if (frontNeighbor != NIL) {
        mergedSize += HEADER_SIZE + getSize(heap, frontNeighbor);
        mergedStart = frontNeighbor;  /* 합친 block은 앞쪽 이웃 자리에서 시작 */
        removeFromList(heap, frontNeighbor);
    }

    /* 4. 합쳐진 block을 free list 맨 앞에 넣는다 */
    setSize(heap, mergedStart, mergedSize);
    setNext(heap, mergedStart, heap->freeHead);
    heap->freeHead = mergedStart;

    heap->lastError = OK;
    heap->stats.freeOk++;
    return OK;
}

/* ---------- getHeapInfo ----------
 * free list를 돌면서 개수, 합계, 가장 큰 크기를 센다.
 */
HeapInfo getHeapInfo(const Heap* heap) {
    HeapInfo info = { 0, 0, 0 };
    int current;

    if (!heap->isReady) return info;
    for (current = heap->freeHead; current != NIL; current = getNext(heap, current)) {
        int size = getSize(heap, current);
        info.freeCount++;
        info.totalFree += size;
        if (size > info.largestFree) info.largestFree = size;
    }
    return info;
}
