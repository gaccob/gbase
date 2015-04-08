#include <assert.h>
#include <pthread.h>
#include <unistd.h>

#include "mm/shm.h"
#include "core/lock.h"

int SHM_KEY = 2100001;
size_t SHM_SIZE = 1024;

static void*
_send(void* arg) {

    uint64_t tid = pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);

    printf("thread[%lx] shm size:%d addr:%p\n", tid, (int)shm_size(shm), (char*)shm_mem(shm));

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", (char*)arg);

    lock_lock(pl);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
    printf("thread[%lx] send: %s\n", tid, buf);
    lock_unlock(pl);

    shm_release(shm);
    lock_release(pl);
    return 0;
}

// only destory by receiver
static void*
_recv(void* arg) {

    uint64_t tid = pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);

    printf("thread[%lx] shm size:%d addr:%p\n", tid, (int)shm_size(shm), (char*)shm_mem(shm));

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    usleep(100);

    lock_lock(pl);
    printf("thread[%lx] recv: %s\n", tid, (char*)shm_mem(shm));
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
test_mm_shm(char* param) {
    char words[1024];
    snprintf(words, sizeof(words), "hello shm!");

    int destroy = 0;
    pthread_t t_send, t_recv, t_destroy;
    pthread_create(&t_send, NULL, _send, words);
    pthread_create(&t_recv, NULL, _recv, NULL);
    pthread_create(&t_destroy, NULL, _recv, &destroy);

    pthread_join(t_send, NULL);
    pthread_join(t_recv, NULL);
    pthread_join(t_destroy, NULL);

    return 0;
}

