#include <assert.h>
#include <string.h>
#include "array.h"

typedef struct array_t {
    int32_t count;
    int32_t size;
    void** array;
} array_t;

array_t*
array_create(int32_t size) {
    array_t* array;
    if (size <= 0) return NULL;
    array = (array_t*)MALLOC(sizeof(array_t));
    if (!array) goto ARRAY_FAIL;

    array->array = (void**)MALLOC(sizeof(void*) * size);
    if (!array->array) goto ARRAY_FAIL1;

    array->size = size;
    array->count = 0;
    assert(array->array);
    memset(array->array, 0, sizeof(void*) * size);
    return array;

ARRAY_FAIL1:
    FREE(array);
ARRAY_FAIL:
    return NULL;
}

int32_t
array_add(array_t* array, void* data) {
    if (!array || !data) return -1;
    if (array->count >= array->size) return -1;
    array->array[array->count] = data;
    array->count ++;
    return 0;
}

int32_t
array_del(array_t* array, void* data) {
    int32_t index;
    if (!array || !data) return -1;
    for (index = 0; index < array->count; index ++) {
        if (array->array[index] == data) {
            array->array[index] = array->array[array->count - 1];
            array->array[array->count - 1] = 0;
            array->count --;
            break;
        }
    }
    return 0;
}

int32_t
array_has(array_t* array, void* data) {
    int32_t index;
    if (!array || !data) return -1;
    for (index = 0; index < array->count; index ++) {
        if (array->array[index] == data) {
            return 0;
        }
    }
    return -1;
}

int32_t
array_count(array_t* array) {
    if (!array) return -1;
    return array->count;
}


int32_t
array_loop(array_t* array, array_loop_func func, void* args) {
    int32_t index;
    if (!array || array->count <= 0) return -1;

    for (index = 0; index < array->count; index ++) {
        func(array->array[index], args);
    }
    return 0;
}

void
array_release(array_t* array) {
    if (array) {
        FREE(array->array);
        FREE(array);
    }
}

