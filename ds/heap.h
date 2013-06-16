#ifndef HEAP_H_
#define HEAP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"

/*
* minimum heap
*/
struct heap_t;
typedef int (*heap_cmp)(void*, void*);

struct heap_t* heap_init(heap_cmp cmp);
void heap_release(struct heap_t* heap);

/*
*    return >= 0, success, return key which used to erase data
*    return < 0, fail
*/
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

