#include <assert.h>
#include "util/random.h"
#include "base/bitset.h"

int
test_base_bitset(const char* param) {
    rand_seed((uint32_t)time(NULL));

    // bit create 
    int size = param ? atoi(param) : rand() % 1024;
    bit_t* bit = bit_create(size);
    if (!bit) {
        fprintf(stderr, "bit create fail\n");
        return -1;
    }

    // bit set 
    int test_size = (size >> 1);
    for (int i = 0; i < test_size; ++ i) {
        bit_set(bit, i);
    }

    // bit is set 
    for (int i = 0; i < size; ++ i) {
        if (i < test_size && bit_isset(bit, i)) {
            fprintf(stderr, "bit-set error\n");
            bit_release(bit);
            return -1;
        }
        if (i >= test_size && bit_isset(bit, i) == 0) {
            fprintf(stderr, "bit-set error\n");
            bit_release(bit);
            return -1;
        }
    }

    // bit count
    if (bit_count(bit) != test_size) {
        fprintf(stderr, "bit-count = %d error\n", bit_count(bit));
        bit_release(bit);
        return -1;
    }

    // bit reset -> bit count
    for (int i = 0; i < test_size; ++ i) {
        bit_reset(bit, i);
    }
    if (bit_count(bit) != 0) {
        fprintf(stderr, "bit-count error\n");
        bit_release(bit);
        return -1;
    }

    bit_release(bit);
    return 0;
}

