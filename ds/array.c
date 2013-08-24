#include <assert.h>
#include <string.h>
#include "ds/array.h"

typedef struct array_t
{
    int32_t count;
    int32_t size;
    void** array;
}array_t;

struct array_t* array_init(int32_t size)
{
    struct array_t* array;
    assert(size > 0);
    array = (struct array_t*)MALLOC(sizeof(struct array_t));
    if(!array)
        goto ARRAY_FAIL;

    array->array = (void**)MALLOC(sizeof(void*) * size);
    if(!array->array)
        goto ARRAY_FAIL1;

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

int32_t array_add(struct array_t* array, void* data)
{
    assert(array);
    if(array->count >= array->size)
        return -1;
    array->array[array->count] = data;
    array->count ++;
    return 0;
}

int32_t array_del(struct array_t* array, void* data)
{
    int32_t index;
    assert(array);
    for(index = 0; index < array->count; index ++)
    {
        if(array->array[index] == data)
        {
            array->array[index] = array->array[array->count - 1];
            array->array[array->count - 1] = 0;
            array->count --;
            break;
        }
    }
    return 0;
}

int32_t array_has(struct array_t* array, void* data)
{
    int32_t index;
    assert(array);
    for(index = 0; index < array->count; index ++)
    {
        if(array->array[index] == data)
        {
            return 0;
        }
    }
    return -1;
}

int32_t array_count(struct array_t* array)
{
    assert(array);
    return array->count;
}


int32_t array_loop(struct array_t* array, array_fn func, void* args)
{
    int32_t index;
    assert(array);
    if(array->count <= 0)
        return -1;

    for(index = 0; index < array->count; index ++)
        func(array->array[index], args);
    return 0;
}

void array_release(struct array_t* array)
{
    if(array)
    {
        FREE(array->array);
        FREE(array);
    }
}

