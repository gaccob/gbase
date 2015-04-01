#include <assert.h>
#include "core/thread.h"
#include "core/lock.h"
#include "util/util_time.h"

static void* _lock;
static int _lock_thread_count;
static int _lock_loop = 10000;
static int _lock_code_len = 1;

static void*
_lock_func(void* arg) {
    int k = rand();
    for (int i = 0; i < _lock_loop; ++ i) {
        thread_lock(_lock);
        for (int l = 0; l < _lock_code_len; ++ l) {
            int j = k;
            int m = j;
            k = m;
            m = j + 1;
            k = m + 2;
            j = m + k;
        }
        thread_unlock(_lock);
    }
    return NULL;
}

int
test_core_lock(char* param) {

    _lock_thread_count = param ? atoi(param) : 4;

    _lock = thread_lock_alloc();
    if (!_lock) {
        fprintf(stderr, "thread lock alloc fail\n");
        return -1;
    }

    pthread_t* tid = (pthread_t*)MALLOC(sizeof(pthread_t) * _lock_thread_count);

    struct timeval from;
    gettimeofday(&from,NULL);
    char ts[128];
    util_timestamp(&from, ts, sizeof(ts));
    printf("\t%s thread %d loop %d start\n", ts, _lock_thread_count, _lock_loop);

    for (int i = 0; i < _lock_thread_count; ++ i) {
        pthread_create(&tid[i], NULL, _lock_func, NULL);
    }
    for (int i = 0; i < _lock_thread_count; ++ i) {
        int ret = pthread_join(tid[i], NULL);
        if (ret !=0) {
            fprintf(stderr, "cannot join thread %d\n", i);
        }
    }

    struct timeval end;
    gettimeofday(&end,NULL);
    util_timestamp(&end, ts, sizeof(ts));
    printf("\t%s thread %d loop %d end\n", ts, _lock_thread_count, _lock_loop);

    FREE(tid);
    thread_lock_free(_lock);
    return 0;
}

