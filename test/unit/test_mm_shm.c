#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <pthread.h>
#include <unistd.h>

#include "mm/shm.h"
#include "core/lock.h"

int SHM_KEY = 2100001;
size_t SHM_SIZE = 1024;

static void*
_send(void* arg) {

    pthread_t tid = pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);
#ifdef __x86_64__
    printf("thread[%"PRIX64"] shm size:%d addr:%p\n", (uint64_t)tid, (int)shm_size(shm), (char*)shm_mem(shm));
#else
    printf("thread[%"PRIX32"] shm size:%d addr:%p\n", (uint32_t)tid, (int)shm_size(shm), (char*)shm_mem(shm));
#endif

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    char buf[1024];
    snprintf(buf, sizeof(buf), "%s", (char*)arg);

    lock_lock(pl);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
#ifdef __x86_64__
    printf("thread[%"PRIX64"] send: %s\n", (uint64_t)tid, buf);
#else
    printf("thread[%"PRIX32"] send: %s\n", (uint32_t)tid, buf);
#endif
    lock_unlock(pl);

    shm_release(shm);
    lock_release(pl);
    return 0;
}

// only destory by receiver
static void*
_recv(void* arg) {

    pthread_t tid = pthread_self();

    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);

#ifdef __x86_64__
    printf("thread[%"PRIX64"] shm size:%d addr:%p\n", (uint64_t)tid, (int)shm_size(shm), (char*)shm_mem(shm));
#else
    printf("thread[%"PRIX32"] shm size:%d addr:%p\n", (uint32_t)tid, (int)shm_size(shm), (char*)shm_mem(shm));
#endif

    struct lock_t* pl = lock_create(SHM_KEY);
    assert(pl);

    usleep(100);

    lock_lock(pl);
#ifdef __x86_64__
    printf("thread[%"PRIX64"] recv: %s\n", (uint64_t)tid, (char*)shm_mem(shm));
#else
    printf("thread[%"PRIX32"] recv: %s\n", (uint32_t)tid, (char*)shm_mem(shm));
#endif
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

