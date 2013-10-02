#ifndef RBTREE_H_
#define RBTREE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct rbtree_t;
typedef int (*rbtree_cmp)(void*, void*);
typedef void (*rbtree_func)(void*);

struct rbtree_t* rbtree_init(rbtree_cmp cmp);
void rbtree_release(struct rbtree_t* tree);
int rbtree_insert(struct rbtree_t* tree, void* data);
void* rbtree_find(struct rbtree_t* tree, void* data);
void* rbtree_delete(struct rbtree_t* tree, void* data);
void rbtree_loop(struct rbtree_t* tree, rbtree_func func);

#ifdef __cplusplus
}
#endif

#endif // RBTREE_H_

