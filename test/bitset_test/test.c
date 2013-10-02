#include "core/bitset.h"
#include <assert.h>

int main()
{
    struct bit_t* bit;
    int i, setbit[4];
    int size = rand() % 1024;
    bit = bit_init(size);
    assert(bit);

    for(i = 0; i < 4; i++)
    {
        setbit[i] = rand() % size;
        printf("set bit @ %d\n", setbit[i]);
        bit_set(bit, setbit[i]);
    }

    for(i = 0; i < size; i++)
    {
        if(i == setbit[0] || i == setbit[1] || i == setbit[2] || i == setbit[3])
            assert(bit_isset(bit, i) == 0);
        else
            assert(bit_isset(bit, i) != 0);
    }

    printf("%d bits is set\n", bit_count(bit));
    for(i = 0; i < 4; i++)
    {
        bit_reset(bit, setbit[i]);
    }
    printf("%d bits is set\n", bit_count(bit));
    
    bit_release(bit);
    getchar();
    return 0;
}

