#include <assert.h>
#include "core/thread.h"
#include "core/spin.h"

#include "test.h"

struct spin_lock_t* spinlock;

int spin_thread_count = 4;
int spin_loop = 1000000;
int spin_code_len = 1;

THREAD_FUNC
spin_func(void* arg) {
    int i, j, l, k, m;
    k = rand();
    for (i=0; i<spin_loop; i++) {
        spin_lock(spinlock);
        for (l=0; l<spin_code_len; l++) {
            j = k;
            m = j;
            k = m;
            m = j+1;
            k = m+2;
            j = m+k;
        }
        spin_unlock(spinlock);
    }
    THREAD_RETURN;
}

int
test_spin() {
    thread_t* tid;
    int i, ret, lasttime;
    struct timeval from;

    spinlock = spin_create();
    assert(spinlock);

    tid = (thread_t*)MALLOC(sizeof(thread_t) * spin_thread_count);
    util_gettimeofday(&from,NULL);

    for (i=0; i<spin_thread_count; i++) {
        THREAD_CREATE(tid[i], spin_func, NULL);
    }
    for (i=0; i<spin_thread_count; i++) {
        ret = THREAD_JOIN(tid[i]);
        if (ret !=0) {
            printf("cannot join thread1");
        }
    }

    lasttime = get_process_time(&from);
    printf("spin lock\t\tspin_loop:%d\tspin_code_len:%d\ttime:%d\n",
        spin_loop, spin_code_len, lasttime);
    FREE(tid);

    spin_release(spinlock);
    return 0;
}

