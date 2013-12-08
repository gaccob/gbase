#ifndef SLAB_H_
#define SLAB_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "core/list.h"

// align with 8 bytes
#define SLAB_ALIGN(sz) (((sz + 7) >> 3) << 3)

//
// for small memory (0, 128)
// for common memory [256, 1024)
// slab not fit for memory > 1024 bytes
// slot will split after alloc, while merge after free if possible
//
#define SLAB_SIZE_MINOR 128
#define SLAB_SIZE_MAX 1024

void* slab_alloc(size_t);
void slab_free(void*);

void slab_debug();

#ifdef __cplusplus
}
#endif

#endif
