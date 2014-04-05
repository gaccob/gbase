#include <stdio.h>
#include <assert.h>
#include "test_shm.h"

int main()
{
    char buf[1024];
    int ret;
    struct lock_t* pl;
    struct shm_t* shm = shm_create(shmkey, size, 1);
    assert(shm);
    printf("shm size:%d addr:%p\n", (int)shm_size(shm), (char*)shm_mem(shm));

    pl = lock_create(shmkey);
    assert(pl);

    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);

    ret = lock_lock(pl);
    assert(0 == ret);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
    printf("shm send complete.\n");
    getchar();
    lock_unlock(pl);

    getchar();
    shm_release(shm);
    lock_release(pl);
    return 0;
}
