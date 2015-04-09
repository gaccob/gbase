#include <assert.h>
#include "core/os_def.h"

#ifdef OS_LINUX
#include "core/coroutine.h"

#define CRT_TEST_STACK_SIZE 4096

static void
_crt_main(crt_t* c, void* arg) {
    char tmp[100];
    tmp[0] = 'a';
    for (int tick = 0; tick < 5; ++ tick) {
        int index = *(int*)(arg);
        printf("coroutine[%d] tick %d\n", crt_current(c), index + tick);
        char dummy;
        printf("stack size: %d\n", (int)(crt_current_stack_top(c) - &dummy));
        crt_yield(c);
    }
    printf("coroutine[%d] finish\n", crt_current(c));
}

int
test_core_coroutine(const char* param) {
    crt_t* c = crt_create(CRT_TEST_STACK_SIZE);
    if (!c) {
        fprintf(stderr, "coroutine create fail\n");
        return -1;
    }

    /////////////////////////////////////////////

    int i = 10;
    int c1 = crt_new(c, _crt_main, &i);
    if (c1 < 0) {
        fprintf(stderr, "coroutine instance create fail\n");
        crt_release(c);
        return -1;
    }

    int j = 100;
    int c2 = crt_new(c, _crt_main, &j);
    if (c2 < 0) {
        fprintf(stderr, "coroutine instance create fail\n");
        crt_release(c);
        return -1;
    }

    while (crt_status(c, c1) && crt_status(c, c2)) {
        crt_resume(c, c1);
        crt_resume(c, c2);
    }

    /////////////////////////////////////////////

    int m = -10;
    int c3 = crt_new(c, _crt_main, &m);
    if (c3 < 0) {
        fprintf(stderr, "coroutine instance create fail\n");
        crt_release(c);
        return -1;
    }

    int n = -100;
    int c4 = crt_new(c, _crt_main, &n);
    if (c4 < 0) {
        fprintf(stderr, "coroutine instance create fail\n");
        crt_release(c);
        return -1;
    }

    while (crt_status(c, c3) && crt_status(c, c4)) {
        crt_resume(c, c3);
        crt_resume(c, c4);
    }

    /////////////////////////////////////////////

    crt_release(c);
    return 0;
}

#endif

