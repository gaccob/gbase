#ifndef BITSET_H_
#define BITSET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

struct bit_t;
struct bit_t* bit_init(int size);
void bit_release(struct bit_t* bit);
void bit_set(struct bit_t* bit, int index);
void bit_reset(struct bit_t* bit, int index);
int bit_isset(struct bit_t* bit, int index);
int bit_count(struct bit_t* bit);

#ifdef __cplusplus
}
#endif

#endif // BITSET_H_


