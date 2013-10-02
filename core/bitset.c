#include "bitset.h"

typedef struct bit_t
{
    unsigned char* data;
    int bit_size;
    int byte_size;
}bit_t;

#define BIT_BYTES(len) ((((len) + 8 - 1) & ( ~ (8 - 1))) / 8)

bit_t* bit_init(int size)
{
    bit_t* bit = (bit_t*)MALLOC(sizeof(bit_t));
    if (!bit) goto BIT_FAIL;

    bit->bit_size = size;
    bit->byte_size = BIT_BYTES(size);
    bit->data = (unsigned char*)MALLOC(sizeof(unsigned char) * bit->byte_size);
    if (!bit->data) goto BIT_FAIL1;
    memset(bit->data, 0, sizeof(unsigned char) * bit->byte_size);
    return bit;

BIT_FAIL1:
    FREE(bit);
BIT_FAIL:
    return NULL;
}

void bit_release(bit_t* bit)
{
    if (bit) {
        FREE(bit->data);
        FREE(bit);
    }
}

void bit_set(bit_t* bit, int index)
{
    if (!bit || index < 0 || index >= bit->bit_size) {
        return;
    }
    bit->data[index / 8] |= (1 << (index % 8));
}

void bit_reset(bit_t* bit, int index)
{
    if (!bit || index < 0 || index >= bit->bit_size) {
        return;
    }
    bit->data[index / 8] &= (~(1 << (index % 8)));
}

int bit_isset(bit_t* bit, int index)
{
    if (!bit || index < 0 || index >= bit->bit_size) {
        return -1;
    }
    return (bit->data[index / 8] & (1 << (index % 8))) ? 0 : -1;
}

int bit_count(bit_t* bit)
{
    int index, sum = 0;
    static int count[16] = { 0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4 };
    if (!bit) {
        return -1;
    }
    for (index = 0; index < bit->byte_size; index ++) {
        sum += (count[bit->data[index] & 0x0f]
            + count[bit->data[index] >> 4]);
    }
    return sum;
}

