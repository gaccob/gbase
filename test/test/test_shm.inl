#include "mm/shm.h"
#include "core/lock.h"

int SHM_KEY = 2100001;
size_t SHM_SIZE = 1024;

int
test_shm_send() {
    char buf[1024];
    int ret;
    struct lock_t* pl;
    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);
    printf("shm size:%d addr:%p\n", (int)shm_size(shm), (char*)shm_mem(shm));

    pl = lock_create(SHM_KEY);
    assert(pl);

    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);

    ret = lock_lock(pl);
    assert(0 == ret);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
    printf("shm send complete.\n");

    lock_unlock(pl);
    shm_release(shm);
    lock_release(pl);
    return 0;
}

// only destory by receiver
int
test_shm_recv() {
    struct lock_t* pl;
    struct shm_t* shm = shm_create(SHM_KEY, SHM_SIZE, 1);
    assert(shm);
    pl = lock_create(SHM_KEY);
    assert(pl);

    printf("shm size:%d addr:%p\n", (int)shm_size(shm), (char*)shm_mem(shm));
    printf("press any key to recv shm buffer: \n");
    getchar();
    printf("recv: %s\n", (char*)shm_mem(shm));

    shm_destroy(shm);
    shm_release(shm);
    lock_destroy(pl);
    lock_release(pl);
    return 0;
}

