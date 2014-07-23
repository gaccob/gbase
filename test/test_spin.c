#include <assert.h>
#include <sys/time.h>
#include "core/thread.h"
#include "core/spin.h"

#include "test.h"

spin_t* spinlock;

int spin_thread_count = 4;
int spin_loop = 1000000;
int spin_code_len = 1;

void*
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
    return NULL;
}

int
test_spin() {
    pthread_t* tid;
    int i, ret, lasttime;
    struct timeval from;

    spinlock = spin_create();
    assert(spinlock);

    tid = (pthread_t*)MALLOC(sizeof(pthread_t) * spin_thread_count);
    gettimeofday(&from,NULL);

    for (i=0; i<spin_thread_count; i++) {
        pthread_create(&tid[i], NULL, spin_func, NULL);
    }
    for (i=0; i<spin_thread_count; i++) {
        ret = pthread_join(tid[i], NULL);
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

