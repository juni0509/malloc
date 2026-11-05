#include<string.h>
#include "manager.h"
#include "memory.h"


ErrorCode initHeap(Heap* h, int size) {
	if (size <HEADER_SIZE + 1 || size>MAX_SIZE) {
		return BAD_SIZE;
	}


	h->heapSize = size;
	h->freeHead = 0;
	h->isReady = 1;
	h->lastError = OK;
	h->nextFitPos= 0;
	memset(&h->stats, 0, sizeof(Stats));


	setSize(h, 0, size - HEADER_SIZE);
	setNext(h, 0, NIL);

	return OK;
}
/* ---------- checkAllocRequest ----------
 * myalloc 요청이 올바른지 본다. 문제없으면 OK.
 * 초기화 여부를 먼저 보고, 그다음에 크기를 본다.
 */
ErrorCode checkAllocRequest(const Heap *h, int n) {
    if (!h->isReady) return NOT_INIT;
    if (n < 1 || n > h->heapSize - HEADER_SIZE) return BAD_SIZE;
    return OK;
}

/* ---------- findFirstFit ----------
 * free list를 앞에서부터 보며 size가 n 이상인 "첫 block"을 찾는다.
 * 돌려주는 값: 찾은 block의 시작 위치 (없으면 NIL)
 * *prev: 찾은 block 바로 앞 free block (맨 앞이면 NIL)
 */
int findFirstFit(Heap *h, int n, int *prev) {
    int p = NIL;
    int cur = h->freeHead;

    while (cur != NIL) {
        h->stats.searchSteps++;
        if (getSize(h, cur) >= n) {
            *prev = p;
            return cur;
        }
        p = cur;
        cur = getNext(h, cur);
    }
    *prev = NIL;
    return NIL;
}
/* ---------- myalloc ----------
 * 하는 일: n bytes를 할당하고, 사용자에게 줄 주소를 돌려준다. 실패하면 -1.
 * 실패 사유는 h->lastError에 적어 둔다.
 */
 /* ---------- findBestFit ----------
  * 맞는 block(size >= n) 중에서 "가장 작은 것"을 고른다.
  * 크기가 같으면 리스트에서 먼저 나온 것을 고른다 (< 로 비교하니까).
  * first-fit과 달리 첫 번째에서 멈추지 않고 끝까지 본다.
  */
int findBestFit(Heap* h, int n, int* prev) {
    int p = NIL, cur = h->freeHead;
    int best = NIL, bestPrev = NIL;      // 지금까지 찾은 최고 후보와 그 앞 block

    while (cur != NIL) {
        h->stats.searchSteps++;
        if (getSize(h, cur) >= n) {
            if (best == NIL || getSize(h, cur) < getSize(h, best)) {
                best = cur;              // 더 작은 후보가 나왔다
                bestPrev = p;
            }
        }
        p = cur;
        cur = getNext(h, cur);
    }
    *prev = bestPrev;
    return best;
}

/* blk 바로 앞 free block을 찾는다 (맨 앞이면 NIL). findNextFit에서만 쓴다. */
static int prevInList(const Heap* h, int blk) {
    int p = NIL, cur = h->freeHead;
    while (cur != NIL && cur != blk) {
        p = cur;
        cur = getNext(h, cur);
    }
    return p;
}

/* ---------- findNextFit ----------
 * 지난번에 할당한 자리(nextFitPos)부터 끝까지 찾고,
 * 없으면 맨 앞에서부터 nextFitPos 직전까지 찾는다. (한 바퀴)
 */
int findNextFit(Heap* h, int n, int* prev) {
    int start = h->nextFitPos;
    int p, cur;

    // 그 block이 합쳐지거나 할당돼서 사라졌다면 맨 앞부터 시작
    if (!inFreeList(h, start)) start = h->freeHead;

    /* 1단계: start부터 끝까지 */
    p = prevInList(h, start);
    cur = start;
    while (cur != NIL) {
        h->stats.searchSteps++;
        if (getSize(h, cur) >= n) { *prev = p; return cur; }
        p = cur;
        cur = getNext(h, cur);
    }

    /* 2단계: 맨 앞부터 start 전까지 */
    p = NIL;
    cur = h->freeHead;
    while (cur != NIL && cur != start) {
        h->stats.searchSteps++;
        if (getSize(h, cur) >= n) { *prev = p; return cur; }
        p = cur;
        cur = getNext(h, cur);
    }

    *prev = NIL;
    return NIL;
}
int myalloc(Heap* h, int n) {
    int prev, blk, rest, newBlk;
    ErrorCode err = checkAllocRequest(h, n);

    /* 1. 요청이 올바른가? */
    if (err != OK) {
        h->lastError = err;
        h->stats.allocFail++;
        return -1;
    }

    /* 2. 맞는 free block을 찾는다 */
    if (h->strategy == BEST_FIT)      blk = findBestFit(h, n, &prev);
    else if (h->strategy == NEXT_FIT) blk = findNextFit(h, n, &prev);
    else                              blk = findFirstFit(h, n, &prev);
    if (blk == NIL) {
        h->lastError = NO_SPACE;
        h->stats.allocFail++;
        return -1;
    }

    /* 3. 쪼갤까, 통째로 줄까? */
    rest = getSize(h, blk) - n;
    if (rest >= HEADER_SIZE + 1) {
        /* 쪼갠다 */
        newBlk = blk + HEADER_SIZE + n;
        setSize(h, newBlk, rest - HEADER_SIZE);
        setNext(h, newBlk, getNext(h, blk));
        if (prev == NIL) h->freeHead = newBlk;
        else             setNext(h, prev, newBlk);
        setSize(h, blk, n);
        h->nextFitPos = newBlk;               // ← ②번 (쪼개는 쪽 맨 끝)
    }
    else {
        h->nextFitPos = getNext(h, blk);      // ← ③번 (여기! else { 바로 아래)
        /* 통째로 준다 */
        if (prev == NIL) h->freeHead = getNext(h, blk);
        else             setNext(h, prev, getNext(h, blk));
        h->stats.wasted += rest;
    }

    /* 4. 마무리 */
    setNext(h, blk, NIL);                     // 사용 중 block은 next = -1
    h->lastError = OK;
    h->stats.allocOk++;
    return blk + HEADER_SIZE;                 // 사용자 주소 = 시작 + 8
}
// manager.c (맨 아래에)

/* free list에서 blk를 뺀다 */
static void removeFromList(Heap* h, int blk) {
    int prev = NIL;
    int cur = h->freeHead;

    while (cur != NIL && cur != blk) {
        prev = cur;
        cur = getNext(h, cur);
    }
    if (cur == NIL) return;                    /* 목록에 없으면 할 일 없음 */

    if (prev == NIL) h->freeHead = getNext(h, cur);
    else             setNext(h, prev, getNext(h, cur));
}

/* 끝이 pos와 딱 맞닿는 free block(앞쪽 이웃)을 찾는다. 없으면 NIL */
static int findFrontNeighbor(const Heap* h, int pos) {
    int cur;
    for (cur = h->freeHead; cur != NIL; cur = getNext(h, cur)) {
        if (cur + HEADER_SIZE + getSize(h, cur) == pos) return cur;
    }
    return NIL;
}

/* ---------- myfree ----------
 * addr(사용자 주소)의 block을 돌려받는다. 성공하면 OK, 아니면 오류 코드.
 */
ErrorCode myfree(Heap* h, int addr) {
    int blk = addr - HEADER_SIZE;
    int start, size, back, front;

    /* 1. 주소 검사 (순서 중요) */
    if (!h->isReady) { h->lastError = NOT_INIT;    return NOT_INIT; }
    if (!isBlockStart(h, blk)) { h->lastError = BAD_ADDRESS; return BAD_ADDRESS; }
    if (inFreeList(h, blk)) { h->lastError = DOUBLE_FREE; return DOUBLE_FREE; }

    start = blk;
    size = getSize(h, blk);

    /* 2. 뒤쪽 이웃: 내 block이 끝나는 자리에서 시작하는 free block */
    back = start + HEADER_SIZE + size;
    if (inFreeList(h, back)) {
        size += HEADER_SIZE + getSize(h, back);
        removeFromList(h, back);
    }

    /* 3. 앞쪽 이웃: 끝이 내 시작과 맞닿는 free block */
    front = findFrontNeighbor(h, start);
    if (front != NIL) {
        size += HEADER_SIZE + getSize(h, front);
        start = front;                         /* 합친 block은 앞쪽 이웃 자리에서 시작 */
        removeFromList(h, front);
    }

    /* 4. 합쳐진 block을 free list 맨 앞에 넣는다 */
    setSize(h, start, size);
    setNext(h, start, h->freeHead);
    h->freeHead = start;

    h->lastError = OK;
    h->stats.freeOk++;
    return OK;
}

/* ---------- getHeapInfo ----------
 * free list를 돌면서 개수, 합계, 가장 큰 크기를 센다.
 */
HeapInfo getHeapInfo(const Heap* h) {
    HeapInfo info = { 0, 0, 0 };
    int cur;

    if (!h->isReady) return info;
    for (cur = h->freeHead; cur != NIL; cur = getNext(h, cur)) {
        int size = getSize(h, cur);
        info.freeCount++;
        info.totalFree += size;
        if (size > info.largestFree) info.largestFree = size;
    }
    return info;
}
