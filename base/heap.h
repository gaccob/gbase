#ifndef HEAP_H_
#define HEAP_H_

// it's a minimum heap, grows automatic

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct heap_t heap_t;
typedef int (*heap_cmp_func)(void*, void*);

heap_t* heap_create(heap_cmp_func cmp);
void heap_release(heap_t* heap);

//  return >= 0, success, return key
//  return < 0, fail
int heap_insert(heap_t* heap, void* data);
void* heap_erase(heap_t* heap, int key);
void heap_update(heap_t* heap, int key, void* data);
int heap_size(heap_t* heap);
void* heap_top(heap_t* heap);
void* heap_pop(heap_t* heap);

#ifdef __cplusplus
}
#endif

#endif // HEAP_H_

