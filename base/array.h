//
// nearly same as single list, but only fixed size
// so, it's an array not list
//
#ifndef ARRAY_H_
#define ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef struct array_t array_t;
typedef void (*array_loop_func)(void* data, void* args);

array_t* array_create(int32_t size);
int32_t array_add(array_t* array, void* data);
int32_t array_del(array_t* array, void* data);
int32_t array_has(array_t* array, void* data);
int32_t array_count(array_t* array);
int32_t array_loop(array_t* array, array_loop_func func, void* args);
void array_release(array_t* array);

#ifdef __cplusplus
}
#endif

#endif // ARRAY_H_

