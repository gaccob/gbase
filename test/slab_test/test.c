#include <assert.h>

#include <mm/slab.h>
#include <util/random.h>
#include <util/util_time.h>

#define LOOP 1000000

int main()
{
    void* mem[LOOP];
    int i;
    int size;
    struct timeval tv;
    char stamp[64];

    util_gettimeofday(&tv, NULL);
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);

    rand_seed(tv.tv_sec);
    for (i = 0; i < LOOP; ++ i) {
        size = rand_gen() % SLAB_SIZE_MAX + 1;
        // printf("alloc %d\n", size);
        mem[i] = slab_alloc(size);
        assert(mem[i]);
        // slab_debug();
    }

    util_gettimeofday(&tv, NULL);
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);

    for (i = 0; i < LOOP; ++ i) {
        slab_free(mem[i]);
        // slab_debug();
    }

    util_gettimeofday(&tv, NULL);
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);
	getchar();
    return 0;
}
