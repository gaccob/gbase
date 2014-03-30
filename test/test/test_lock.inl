#include "core/process_lock.h"

#include "test.h"

void* threadlock;

int lock_thread_count = 4;
int lock_loop = 1000000;
int lock_code_len = 1;

THREAD_FUNC
lock_func(void* arg) {
    int i, j, l, k, m;
    k = rand();
    for (i=0; i<lock_loop; i++) {
        thread_lock(threadlock);
        for (l=0; l<lock_code_len; l++) {
            j = k;
            m = j;
            k = m;
            m = j+1;
            k = m+2;
            j = m+k;
        }
        thread_unlock(threadlock);
    }
    THREAD_RETURN;
}

void
test_lock() {
    threadlock = thread_lock_alloc();
    assert(threadlock);

    thread_t* tid;
    int i, ret, lasttime;
    struct timeval from;
    tid = (thread_t*)MALLOC(sizeof(thread_t) * lock_thread_count);
    util_gettimeofday(&from,NULL);

    for (i=0; i<lock_thread_count; i++) {
        THREAD_CREATE(tid[i], lock_func, NULL);
    }
    for (i=0; i<lock_thread_count; i++) {
        ret = THREAD_JOIN(tid[i]);
        if (ret !=0) {
            printf("cannot join thread1");
        }
    }

    lasttime = get_process_time(&from);
    printf("thread lock\t\tlock_loop:%d\tlock_code_len:%d\ttime:%d\n",
        lock_loop, lock_code_len, lasttime);
    FREE(tid);

    thread_lock_free(threadlock);
}

