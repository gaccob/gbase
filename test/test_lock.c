#include <assert.h>
#include "core/thread.h"
#include "core/lock.h"

#include "test.h"

void* threadlock;

int lock_thread_count = 4;
int lock_loop = 1000000;
int lock_code_len = 1;

void*
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
    return NULL;
}

int
test_lock() {
    pthread_t* tid;
    int i, ret, lasttime;
    struct timeval from;

    threadlock = thread_lock_alloc();
    assert(threadlock);

    tid = (pthread_t*)MALLOC(sizeof(pthread_t) * lock_thread_count);
    gettimeofday(&from,NULL);

    for (i = 0; i < lock_thread_count; ++ i) {
        pthread_create(&tid[i], NULL, lock_func, NULL);
    }
    for (i = 0; i < lock_thread_count; ++ i) {
        ret = pthread_join(tid[i], NULL);
        if (ret !=0) {
            printf("cannot join thread1");
        }
    }

    lasttime = get_process_time(&from);
    printf("thread lock\t\tlock_loop:%d\tlock_code_len:%d\ttime:%d\n",
        lock_loop, lock_code_len, lasttime);
    FREE(tid);

    thread_lock_free(threadlock);
    return 0;
}

