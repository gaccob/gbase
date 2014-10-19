#ifndef SKIP_LIST_H_
#define SKIP_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#define MAX_SKIPLIST_LEVEL 4
typedef int (*skiplist_cmp_func)(void*, void*);

typedef struct skiplist_t skiplist_t;
skiplist_t* skiplist_create(skiplist_cmp_func cmp);
void skiplist_release(skiplist_t* sl);
int skiplist_insert(skiplist_t* sl, void* data);
void* skiplist_find(skiplist_t* sl, void* data, int erase);

typedef const char* (*skiplist_tostring_func)(void*);
void skiplist_debug(skiplist_t* sl, skiplist_tostring_func tostring);

#ifdef __cplusplus
}
#endif

#endif
