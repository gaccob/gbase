#ifndef HEAP_H_
#define HEAP_H_

// it's a minimum heap, grows automatic

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct heap_t;
typedef int (*heap_cmp_func)(void*, void*);

struct heap_t* heap_create(heap_cmp_func cmp);
void heap_release(struct heap_t* heap);

//  return >= 0, success, return key
//  return < 0, fail
int heap_insert(struct heap_t* heap, void* data);
void* heap_erase(struct heap_t* heap, int key);
void heap_update(struct heap_t* heap, int key, void* data);
int heap_count(struct heap_t* heap);
void* heap_top(struct heap_t* heap);
void* heap_pop(struct heap_t* heap);


#ifdef __cplusplus
}
#endif

#endif // HEAP_H_

