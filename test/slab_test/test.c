#include <assert.h>

#include <mm/slab.h>
#include <util/random.h>

#define LOOP 50

int main()
{
    void* mem[LOOP];
    int i;
    int size;
    
    rand_seed(time(NULL));
    for (i = 0; i < LOOP; ++ i) {
        size = rand_gen() % SLAB_SIZE_MAX + 1;
        printf("alloc %d\n", size);
        mem[i] = slab_alloc(size);
        assert(mem[i]);
        slab_debug();
    }

    for (i = 0; i < LOOP; ++ i) {
        slab_free(mem[i]);
        slab_debug();
    }

    return 0;
}
