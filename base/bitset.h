#ifndef BITSET_H_
#define BITSET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct bit_t bit_t;

bit_t* bit_create(int size);
void bit_release(bit_t* bit);
void bit_set(bit_t* bit, int index);
void bit_reset(bit_t* bit, int index);
int bit_isset(bit_t* bit, int index);
int bit_count(bit_t* bit);

#ifdef __cplusplus
}
#endif

#endif // BITSET_H_

