#ifndef SLAB_H_
#define SLAB_H_
//
// slab requires memory in page size from OS, and organize by list,
// in order to reduce memory fragment
//
// allocation align by 8 byes
// if < SLAB_SIZE_MINOR, we use small slab, 
// else if < SLAB_SIZE_MAX, we use common slab,
// else, slab refuse to allocate
//
// thread unsafe as we use global list head
//
#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "base/list.h"

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
