#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "mm/slab.h"
#include "util/random.h"
#include "util/util_time.h"

#define SLAB_LOOP 10

int
test_slab() {
    void* mem[SLAB_LOOP];

    struct timeval tv;
    gettimeofday(&tv, NULL);

    char stamp[64];
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);

    rand_seed(tv.tv_sec);
    for (int i = 0; i < SLAB_LOOP; ++ i) {
        int size = rand_gen() % SLAB_SIZE_MAX + 1;
        printf("alloc %d\n", size);
        mem[i] = slab_alloc(size);
        assert(mem[i]);
        slab_debug();
    }

    gettimeofday(&tv, NULL);
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);

    for (int i = 0; i < SLAB_LOOP; ++ i) {
        slab_free(mem[i]);
        slab_debug();
    }

    gettimeofday(&tv, NULL);
    util_timestamp(&tv, stamp, 64);
    printf("%s\n", stamp);
    return 0;
}
