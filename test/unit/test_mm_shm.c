#include <assert.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "mm/shm.h"
#include "core/lock.h"

int SHM_KEY = 2100001;
size_t SHM_SIZE = 1024;

static void*
_send(void* arg) {

    uint64_t tid = (uint64_t)pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);

    printf("thread[%llx] shm size:%d addr:%p\n", tid, (int)shm_size(shm), (char*)shm_mem(shm));

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", (char*)arg);

    lock_lock(pl);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
    printf("thread[%llx] send: %s\n", tid, buf);
    lock_unlock(pl);

    shm_release(shm);
    lock_release(pl);
    return 0;
}

// only destory by receiver
static void*
_recv(void* arg) {

    uint64_t tid = (uint64_t)pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);

    printf("thread[%llx] shm size:%d addr:%p\n", tid, (int)shm_size(shm), (char*)shm_mem(shm));

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    usleep(100);

    lock_lock(pl);
    printf("thread[%llx] recv: %s\n", tid, (char*)shm_mem(shm));
    lock_unlock(pl);

    if (arg) {
        int destroy = atoi(arg);
        if (destroy == 0) {
            shm_destroy(shm);
            lock_destroy(pl);
        }
    }

    lock_release(pl);
    shm_release(shm);
    return 0;
}

int
test_mm_shm(const char* param) {
    char words[1024];
    snprintf(words, sizeof(words), "hello shm!");

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 << 20));

    int destroy = 0;
    pthread_t t_send, t_recv, t_destroy;
    pthread_create(&t_send, &attr, _send, words);
    pthread_create(&t_recv, &attr, _recv, NULL);
    pthread_create(&t_destroy, &attr, _recv, &destroy);
    pthread_attr_destroy(&attr);

    pthread_join(t_send, NULL);
    pthread_join(t_recv, NULL);
    pthread_join(t_destroy, NULL);

    return 0;
}

