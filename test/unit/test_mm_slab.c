#include <assert.h>
#include <unistd.h>
#include <sys/time.h>

#include "mm/slab.h"
#include "util/random.h"

static int _loop;

int
test_mm_slab(char* param) {
    _loop = param ? atoi(param) : 10;
    void* mem[_loop];
    memset(mem, 0, sizeof(mem));

    // random alloc or free
    rand_seed(time(NULL));
    for (int i = 0; i < (_loop << 3); ++ i) {
        int idx = rand() % _loop;
        // alloc
        if (mem[idx] == NULL) {
            int size;
            if (rand() & 0x1) {
                size = rand_gen() % SLAB_SIZE_MAX + 1;
            } else {
                size = rand_gen() % SLAB_SIZE_MINOR + 1;
            }
            mem[idx] = slab_alloc(size);
            if (!mem[idx]) {
                fprintf(stderr, "slab alloc fail\n");
                for (int j = 0; j < _loop; ++ j) {
                    if (mem[j]) {
                        slab_free(mem[j]);
                    }
                }
                return -1;
            }
            printf("\talloc array[%d] %d size\n", idx, size);
        }
        // free
        else {
            slab_free(mem[idx]);
            mem[idx] = NULL;
            printf("\tfree array[%d]\n", idx);
        }
    }
    slab_debug();

    // free all
    for (int i = 0; i < _loop; ++ i) {
        slab_free(mem[i]);
    }
    slab_debug();
    return 0;
}

