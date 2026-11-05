#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "types.h"
#include "manager.h"
#include "print.h"
#include "scenario.h"

#define MAX_HANDLES 32

/* 이름 -> 주소 표 한 칸 */
typedef struct {
    char name[9];  /* 최대 8자 + 끝 표시 */
    int addr;
} Handle;

/* 프로그램이 쓰는 모든 것을 한 곳에 모은다 (전역 변수 대신) */
typedef struct {
    Heap heap;
    Handle handles[MAX_HANDLES];
    int handleCount;
    int step;  /* 동작 번호 [1], [2], ... */
} App;

static const char* strategyName(int strategy) {
    if (strategy == BEST_FIT) return "best-fit";
    if (strategy == NEXT_FIT) return "next-fit";
    return "first-fit";
}

/* ---------- 입력 도우미 ---------- */

/* 한 줄을 읽는다. 입력이 끝났으면 0 */
static int readLine(const char* prompt, char* buf, int size) {
    printf("%s", prompt);
    if (fgets(buf, size, stdin) == NULL) return 0;
    buf[strcspn(buf, "\r\n")] = '\0';
    return 1;
}

/* 문자열을 정수로 바꾼다. 성공하면 1 */
static int parseInt(const char* text, int* out) {
    char* end;
    long value = strtol(text, &end, 10);

    if (end == text) return 0;  /* 숫자가 하나도 없음 */
    while (*end == ' ') end++;
    if (*end != '\0') return 0;  /* 숫자 뒤에 다른 글자가 있음 */
    if (value > 1000000L || value < -1000000L) return 0;
    *out = (int)value;
    return 1;
}

/* 정수를 입력받는다. 1: 성공, 0: 숫자가 아님, -1: 입력 끝 */
static int inputInt(const char* prompt, int* out) {
    char buf[64];
    if (!readLine(prompt, buf, sizeof(buf))) return -1;
    return parseInt(buf, out);
}

/* 이름 규칙: 영문자/숫자 1~8자 */
static int isValidName(const char* name) {
    int i, len = (int)strlen(name);

    if (len < 1 || len > 8) return 0;
    for (i = 0; i < len; i++) {
        if (!isalnum((unsigned char)name[i])) return 0;
    }
    return 1;
}

/* ---------- 이름 표 ---------- */
static int findHandle(const App* app, const char* name) {
    int i;
    for (i = 0; i < app->handleCount; i++) {
        if (strcmp(app->handles[i].name, name) == 0) return i;
    }
    return -1;
}

static int findHandleByAddr(const App* app, int addr) {
    int i;
    for (i = 0; i < app->handleCount; i++) {
        if (app->handles[i].addr == addr) return i;
    }
    return -1;
}

static void addHandle(App* app, const char* name, int addr) {
    Handle* entry = &app->handles[app->handleCount];
    int i;
    for (i = 0; i < 8 && name[i] != '\0'; i++) entry->name[i] = name[i];
    entry->name[i] = '\0';
    entry->addr = addr;
    app->handleCount++;
}

static void removeHandle(App* app, int index) {
    int i;
    for (i = index; i < app->handleCount - 1; i++) {
        app->handles[i] = app->handles[i + 1];
    }
    app->handleCount--;
}

/* ---------- 메뉴 동작 ---------- */

/* 1. heap 초기화 */
static void menuInit(App* app) {
    int size, inputResult;

    while (1) {
        inputResult = inputInt("SIZE (9 ~ 4096) > ", &size);
        if (inputResult == -1) return;
        if (inputResult == 1 && initHeap(&app->heap, size) == OK) break;
        printf("9 이상 4096 이하의 정수를 입력하세요.\n");
    }
    app->handleCount = 0;
    app->step = 0;
    printf("heap 초기화: SIZE=%d\n    ", size);
    printFreeList(&app->heap);
}

/* 2. 할당 */
static void menuAlloc(App* app) {
    char name[64];
    int requestSize, addr;
    HeapInfo info;

    if (!app->heap.isReady) {
        printf("heap이 초기화되지 않았습니다 (%s)\n", errorText(NOT_INIT));
        return;
    }
    if (!readLine("이름 (영문자/숫자 1~8자) > ", name, sizeof(name))) return;
    if (!isValidName(name)) {
        printf("이름은 영문자/숫자 1~8자만 가능합니다.\n");
        return;
    }
    if (findHandle(app, name) >= 0) {
        printf("이미 쓰는 이름입니다: %s\n", name);
        return;
    }
    if (app->handleCount >= MAX_HANDLES) {
        printf("이름표가 가득 찼습니다. 먼저 반환하세요.\n");
        return;
    }
    if (inputInt("크기 (bytes) > ", &requestSize) != 1) {
        printf("숫자를 입력하세요.\n");
        return;
    }

    app->step++;
    addr = myalloc(&app->heap, requestSize);
    if (addr >= 0) {
        addHandle(app, name, addr);
        printf("[%d] %s = myalloc(%d) -> %d\n", app->step, name, requestSize, addr);
    }
    else {
        printf("[%d] %s = myalloc(%d) -> -1 (실패: %s)\n",
            app->step, name, requestSize, errorText(app->heap.lastError));
        if (app->heap.lastError == NO_SPACE) {
            info = getHeapInfo(&app->heap);
            printf("    할당 실패: 요청 %d, 가장 큰 free block %d, free 합계 %d\n",
                requestSize, info.largestFree, info.totalFree);
        }
    }
    printf("    ");
    printFreeList(&app->heap);
}

/* 3. 반환 (이름 또는 주소) */
static void menuFree(App* app) {
    char text[64];
    int handleIndex, addr;
    ErrorCode result;

    if (!readLine("반환할 이름 또는 주소 > ", text, sizeof(text))) return;

    handleIndex = findHandle(app, text);
    if (handleIndex >= 0) {
        addr = app->handles[handleIndex].addr;
    }
    else if (!parseInt(text, &addr)) {
        printf("없는 이름이거나 올바른 주소가 아닙니다.\n");
        return;
    }

    app->step++;
    result = myfree(&app->heap, addr);
    printf("[%d] myfree(%s) -> %s\n", app->step, text, errorText(result));
    if (result == OK) {
        handleIndex = findHandleByAddr(app, addr);
        if (handleIndex >= 0) removeHandle(app, handleIndex);
    }
    printf("    ");
    printFreeList(&app->heap);
}

/* 4. 상태 출력 */
static void menuStatus(const App* app) {
    HeapInfo info;
    int i;

    printf("\n  -- 현재 상태 --\n");
    printf("  free list : ");
    printFreeList(&app->heap);
    if (!app->heap.isReady) return;

    printHeapMap(&app->heap);
    info = getHeapInfo(&app->heap);
    printf("  ------------------------------\n");
    printf("  free block %d개  |  free 합계 %d  |  가장 큰 free block %d\n",
        info.freeCount, info.totalFree, info.largestFree);
    printf("  이름표 :");
    if (app->handleCount == 0) printf(" (없음)");
    for (i = 0; i < app->handleCount; i++) {
        printf(" %s=%d", app->handles[i].name, app->handles[i].addr);
    }
    printf("\n");
}

/* 6. 전략 선택 */
static void menuStrategy(App* app) {
    int selected;
    printf("1. first-fit  2. best-fit  3. next-fit\n");
    if (inputInt("선택 > ", &selected) != 1 || selected < 1 || selected > 3) {
        printf("1~3 중에서 고르세요.\n");
        return;
    }
    app->heap.strategy = selected - 1;
    printf("전략: %s\n", strategyName(app->heap.strategy));
}

/* 시나리오 번호를 고른다. 잘못되면 -1 */
static int chooseScenario(void) {
    int i, selected;
    for (i = 0; i < caseCount(); i++) {
        printf("  %d. %s\n", i + 1, caseTitle(i));
    }
    if (inputInt("시나리오 번호 > ", &selected) != 1 || selected < 1 || selected > caseCount()) {
        printf("올바른 번호가 아닙니다.\n");
        return -1;
    }
    return selected - 1;
}

/* 7. 시나리오 실행 (지금 heap은 건드리지 않고 따로 돌린다) */
static void menuRunScenario(const App* app) {
    Heap heap = { 0 };
    int id = chooseScenario();

    if (id < 0) return;
    heap.strategy = app->heap.strategy;
    printf("\n[%s / %s]\n", caseTitle(id), strategyName(heap.strategy));
    runCase(&heap, id, 1);
    printHeapMap(&heap);
}

/* 8. 전략 비교 */
static void menuCompare(void) {
    int id = chooseScenario();
    if (id < 0) return;
    printf("\n");
    compareFirstBest(id);
}

/* ---------- main ---------- */
int main(void) {
    App app = { 0 };
    Heap demo = { 0 };
    int choice, inputResult;

    printf("=== 계획서 예시 시나리오 자동 실행 ===\n");
    runCase(&demo, 0, 1);

    while (1) {
        printf("\n ============================================\n");
        printf("   MEMORY MANAGER  |  전략: %s\n", strategyName(app.heap.strategy));
        printf(" ============================================\n");
        printf("   [기본]  1 초기화    2 할당      3 반환    4 상태\n");
        printf("   [실험]  6 전략선택  7 시나리오  8 first/best 비교\n");
        printf("   [끝]    5 종료\n");
        printf(" --------------------------------------------\n");

        inputResult = inputInt(" 선택 > ", &choice);
        if (inputResult == -1) break;
        if (inputResult == 0) {
            printf("메뉴에 있는 번호만 입력하세요.\n");
            continue;
        }
        if (choice == 5) break;

        switch (choice) {
        case 1: menuInit(&app);         break;
        case 2: menuAlloc(&app);        break;
        case 3: menuFree(&app);         break;
        case 4: menuStatus(&app);       break;
        case 6: menuStrategy(&app);     break;
        case 7: menuRunScenario(&app);  break;
        case 8: menuCompare();          break;
        default: printf("메뉴에 있는 번호만 입력하세요.\n");
        }
    }
    printf("종료합니다.\n");
    return 0;
}