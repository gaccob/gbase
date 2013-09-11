#ifndef RANDOM_H_
#define RANDOM_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdlib.h>

void random_seed();
void random_generate(const void* buf, size_t len);

#ifdef __cplusplus
}
#endif

#endif // RANDOM_H_
