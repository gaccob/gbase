#include <assert.h>
#include <sys/time.h>
#include "util/util_time.h"
#include "core/thread.h"
#include "core/spin.h"

spin_t* spinlock;
int spin_loop = 10000;
int spin_code_len = 1;

static void*
_spin_func(void* arg) {
    int i, j, l, k, m;
    k = rand();
    for (i = 0; i < spin_loop; ++ i) {
        spin_lock(spinlock);
        for (l = 0; l < spin_code_len; ++ l) {
            j = k;
            m = j;
            k = m;
            m = j + 1;
            k = m + 2;
            j = m + k;
        }
        spin_unlock(spinlock);
    }
    return NULL;
}

int
test_core_spin(const char* param) {
    spinlock = spin_create();
    if (!spinlock) {
        fprintf(stderr, "spin lock create fail\n");
        return -1;
    }

    int count = param ? atoi(param) : 4;
    pthread_t* tid = (pthread_t*)MALLOC(sizeof(pthread_t) * count);

    struct timeval from;
    gettimeofday(&from,NULL);
    char stamp[64];
    util_timestamp(&from, stamp, sizeof(stamp));
    printf("start: %s\n", stamp);

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 << 20));

    for (int i = 0; i < count; ++ i) {
        pthread_create(&tid[i], &attr, _spin_func, NULL);
    }
    for (int i = 0; i < count; ++ i) {
        int ret = pthread_join(tid[i], NULL);
        if (ret !=0) {
            fprintf(stderr, "cannot join thread[%d]\n", i);
        }
    }
    pthread_attr_destroy(&attr);

    printf("test loop=%d code len=%d\n", spin_loop, spin_code_len);

    struct timeval to;
    gettimeofday(&to,NULL);
    util_timestamp(&to, stamp, sizeof(stamp));
    printf("end: %s\n", stamp);

    FREE(tid);
    spin_release(spinlock);
    return 0;
}

