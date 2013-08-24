/*
*
*  buddy allocator, and with a bin-tree index:
*     loop the tree when do "alloc" until find available
*     merge tree-brother to upper level when du "free".
*     if we get 1<<n size memory, min unit size 1<<m, then we get 1<<(n-m) units
*     we build an index tree, 1 + 2 + 4 + ... + 1<<(n-m) = 1<<(n-m+1) - 1
*     that's why index count = "buddy->pool_size * 2 / buddy->min_size "
*     index shows different buddy status:
*       index[0]-->reserved
*       index[1]-->[0,size)
*       index[2]-->[0,size/2)  index[3]-->(size/2, size)
*       index[4]-->[0,size/4)  index[5]-->(size/4, size/2)  index[6]-->(size/2, size*3/4) index[7]-->(size*3/4, size)
*       .....
*
*    advantage: reduce memory fragment, quick(bin-tree, Olog(N))
*    shortage:  fixed size, and if pool_size/min_unit_size big, the index tree will take lots of addition memory.
*
*    test result:
*        100,000 allocates, cost 7.57us per alloc
*        test machine: 2G cpu * 1, 2G memory, centos linux 2.6.18
*
*  gaccob  2013-01-08
*  reference: http://blog.codingnow.com/2011/12/buddy_memory_allocation.html
*
*/
#ifndef BUDDY_H_
#define BUDDY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct buddy_t;

struct buddy_t* buddy_init(size_t size, size_t min_alloc_size);
int buddy_release(struct buddy_t* buddy);
void* buddy_alloc(struct buddy_t* buddy, size_t nbytes);
void* buddy_realloc(struct buddy_t* buddy, void* mem, size_t nbytes);
void buddy_free(struct buddy_t* buddy, void* mem);
void buddy_debug(struct buddy_t* buddy);

#ifdef __cplusplus
}
#endif

#endif // BUDDY_H_

