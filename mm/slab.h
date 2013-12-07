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
// for small memory (0, 256)
// for medium memory [256, 1024)
// for large memory [1024, pagesize() - reserved head)
// slot will split after alloc, while merge after free if possible
//
#define SLAB_SIZE_SMALL 256
#define SLAB_SIZE_MEDIUM 1024

void* slab_alloc(size_t);
void slab_free(void*);

#ifdef __cplusplus
}
#endif

#endif
