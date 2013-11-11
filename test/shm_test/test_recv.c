#include <assert.h>
#include "test_shm.h"

int main()
{
    struct shm_t* shm = shm_create(shmkey, size);
    assert(shm);
    printf("shm size:%d addr:%p\n", (int)shm_size(shm), (char*)shm_mem(shm));
    printf("press any key to recv shm buffer: \n");

    getchar();
    printf("recv: %s\n", (char*)shm_mem(shm));

    shm_release(shm);
    return 0;
}
