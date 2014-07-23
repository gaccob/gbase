#ifndef RBTREE_H_
#define RBTREE_H_

//
// it's a red-black tree (like a hash table)
// not key-value
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct rbtree_t rbtree_t;
typedef int (*rbtree_cmp_func)(void*, void*);
typedef void (*rbtree_loop_func)(void*);

rbtree_t* rbtree_create(rbtree_cmp_func cmp);
void rbtree_release(rbtree_t* tree);
int rbtree_insert(rbtree_t* tree, void* data);
void* rbtree_find(rbtree_t* tree, void* data);
void* rbtree_delete(rbtree_t* tree, void* data);
void rbtree_loop(rbtree_t* tree, rbtree_loop_func func);

#ifdef __cplusplus
}
#endif

#endif // RBTREE_H_

