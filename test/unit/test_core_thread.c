#include <assert.h>
#include <unistd.h>
#include "core/thread.h"

static int _tval;
static int _loop;
static void* _lock;
static void* _cond;

static void*
_main_zero(void* arg) {
    while (1) {
        thread_lock(_lock);
        while (_tval <= 2) {
            printf("thread[zero] --> %d, wait for wake up\n", _tval);
            thread_cond_wait(_cond, _lock, NULL);
        }
        printf("therad[zero] --> %d, zero it and unlock\n\n", _tval);
        _tval = 0;
        if (-- _loop <= 0) {
            thread_unlock(_lock);
            break;
        }
        thread_unlock(_lock);
    }
    return NULL;
}

static void*
_main_add(void* arg) {
    while (1) {
        thread_lock(_lock);
        if (_loop <= 0) {
            thread_unlock(_lock);
            break;
        }
        ++ _tval;
        thread_unlock(_lock);
        thread_cond_signal(_cond, 0);
        printf("thread[add] --> %d and wake up thread[zero]\n", _tval);
        usleep(100000);
    }
    return NULL;
}

int
test_core_thread(char* param) {
    _cond = thread_cond_alloc();
    _lock = thread_lock_alloc();
    _tval = 0;
    _loop = param ? atoi(param) : 3;

    pthread_t t_add, t_zero;
    pthread_create(&t_add, NULL, _main_add, NULL);
    pthread_create(&t_zero, NULL, _main_zero, NULL);

    pthread_join(t_add, NULL);
    pthread_join(t_zero, NULL);

    thread_cond_free(_cond);
    thread_lock_free(_lock);
    return 0;
}

