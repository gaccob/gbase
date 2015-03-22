#ifndef RANDOM_H_
#define RANDOM_H_

// random algorithm: MT19937

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void rand_seed(uint32_t seed);
uint32_t rand_gen();

// shuffle card
void rand_shuffle(int* src, int sz);

#ifdef __cplusplus
}
#endif

#endif // RANDOM_H_
