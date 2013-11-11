#include <stdio.h>
#include <assert.h>
#include "test_shm.h"

int main()
{
    char buf[1024];
    struct shm_t* shm = shm_create(shmkey, size);
    assert(shm);
    printf("shm size:%d addr:%p\n", (int)shm_size(shm), (char*)shm_mem(shm));

    memset(buf, 0, sizeof(buf));
    fgets(buf, sizeof(buf), stdin);
    memcpy((char*)shm_mem(shm), buf, strlen(buf));
    printf("shm send complete.\n");

    getchar();
    shm_destroy(shm);
    shm_release(shm);
    return 0;
}
