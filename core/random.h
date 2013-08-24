#ifndef RANDOM_H_
#define RANDOM_H_

#ifndef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void random_seed();

void random_generate(const void* buf, size_t len);

#ifndef __cplusplus
}
#endif

#endif // RANDOM_H_
