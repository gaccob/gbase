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

struct rbtree_t;
typedef int (*rbtree_cmp)(void*, void*);
typedef void (*rbtree_loop_func)(void*);

struct rbtree_t* rbtree_create(rbtree_cmp cmp);
void rbtree_release(struct rbtree_t* tree);
int rbtree_insert(struct rbtree_t* tree, void* data);
void* rbtree_find(struct rbtree_t* tree, void* data);
void* rbtree_delete(struct rbtree_t* tree, void* data);
void rbtree_loop(struct rbtree_t* tree, rbtree_loop_func func);

#ifdef __cplusplus
}
#endif

#endif // RBTREE_H_

