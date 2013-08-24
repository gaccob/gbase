#ifndef ARRAY_H_
#define ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

struct array_t;
typedef void (*array_fn)(void* data, void* args);

struct array_t* array_init(int32_t size);
int32_t array_add(struct array_t* array, void* data);
int32_t array_del(struct array_t* array, void* data);
int32_t array_has(struct array_t* array, void* data);
int32_t array_count(struct array_t* array);
int32_t array_loop(struct array_t* array, array_fn func, void* args);
void array_release(struct array_t* array);

#ifdef __cplusplus
}
#endif

#endif // ARRAY_H_

