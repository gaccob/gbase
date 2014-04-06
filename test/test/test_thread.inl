#include "core/thread.h"

int THREAD_VAL = 0;
void* THREAD_LOCK;
void* THREAD_COND;

THREAD_FUNC
thread_zero_run(void *arg) {
    while (1) {
        thread_lock(THREAD_LOCK);
        while (THREAD_VAL <= 2) {
            fprintf(stderr, "thread_zero_run --> THREAD_VAL:%d, wait for wake up\n", THREAD_VAL);
            thread_cond_wait(THREAD_COND, THREAD_LOCK, NULL);
        }
        fprintf(stderr, "therad_zero_run --> THREAD_VAL:%d, zero it and unlock\n", THREAD_VAL);
        THREAD_VAL = 0;
        thread_unlock(THREAD_LOCK);
    }
    THREAD_RETURN;
 }

THREAD_FUNC
thread_add_run(void *arg) {
    while (1) {
        thread_lock(THREAD_LOCK);
        ++ THREAD_VAL;
        thread_unlock(THREAD_LOCK);
        thread_cond_signal(THREAD_COND, 0);
        fprintf(stderr, "after add THREAD_VAL:%d and wake up one zero thread for check\n", THREAD_VAL);
        SLEEP(1000);
    }
    THREAD_RETURN;
}

int
test_thread() {
    thread_t t_add, t_zero;
    THREAD_COND = thread_cond_alloc();
    THREAD_LOCK = thread_lock_alloc();
    THREAD_CREATE(t_add, thread_add_run, NULL);
    THREAD_CREATE(t_zero, thread_zero_run, NULL);
    assert(t_add && t_zero);

    THREAD_JOIN(t_add);
    THREAD_JOIN(t_zero);

    thread_cond_free(THREAD_COND);
    thread_lock_free(THREAD_LOCK);
    return 0;
}

