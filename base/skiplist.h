#ifndef SKIP_LIST_H_
#define SKIP_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

// rank started from 1

#define MAX_SKIPLIST_LEVEL 8

typedef int (*skiplist_cmp_func)(void*, void*);

typedef int (*skiplist_filter_func)(void*, void* args);

typedef struct skiplist_t skiplist_t;

skiplist_t* skiplist_create(skiplist_cmp_func cmp, int level_coff);

void skiplist_release(skiplist_t* sl);

int skiplist_insert(skiplist_t* sl, void* data);

// rank: output data index rank
void* skiplist_find(skiplist_t*, void* data, int* rank);

void* skiplist_erase(skiplist_t*, void* data);

void* skiplist_find_by_rank(skiplist_t* sl, int rank);

void* skiplist_find_from_rank_forward(skiplist_t*, int rank, skiplist_filter_func, void* arg);

void* skiplist_find_from_rank_backward(skiplist_t*, int rank, skiplist_filter_func, void* arg);

// span: in & out
int skiplist_find_list_by_rank(skiplist_t* sl, int rank, int* scope, void** list);

typedef const char* (*skiplist_tostring_func)(void*);
void skiplist_debug(skiplist_t* sl, skiplist_tostring_func tostring);

#ifdef __cplusplus
}
#endif

#endif
