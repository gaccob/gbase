#include <assert.h>
#include "core/os_def.h"

#ifdef OS_LINUX
#include "core/coroutine.h"

#define CRT_TEST_STACK_SIZE 4096

void
crt_func(struct crt_t* c, void* arg) {
    int32_t index = *(int32_t*)(arg);
    int32_t tick;
    char tmp[100];
    tmp[0] = 'a';
    for (tick = 0; tick < 5; ++ tick) {
        printf("coroutine[%d] tick %d\n", crt_current(c), index + tick);
        char dummy;
        printf("stack size: %d\n", (int)(crt_current_stack_top(c) - &dummy));
        crt_yield(c);
    }
    printf("coroutine[%d] finish\n", crt_current(c));
}

int32_t
test_coroutine() {
    int i, j, m, n;
    int c1, c2, c3, c4;
    struct crt_t* c = crt_create(CRT_TEST_STACK_SIZE);
    assert(c);

    i = 10;
    c1 = crt_new(c, crt_func, &i);
    assert(c1 >= 0);
    j = 100;
    c2 = crt_new(c, crt_func, &j);
    assert(c2 >= 0);
    while (crt_status(c, c1) && crt_status(c, c2)) {
        crt_resume(c, c1);
        crt_resume(c, c2);
    }

    m = -10;
    c3 = crt_new(c, crt_func, &m);
    assert(c3 >= 0);
    n = -100;
    c4 = crt_new(c, crt_func, &n);
    assert(c4 >= 0);
    while (crt_status(c, c3) && crt_status(c, c4)) {
        crt_resume(c, c3);
        crt_resume(c, c4);
    }

    crt_release(c);
    return 0;
}

#endif

