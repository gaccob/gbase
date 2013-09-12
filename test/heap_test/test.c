#include "ds/heap.h"
#include <assert.h>

int heap_cmp_func(void* data1, void* data2)
{
    assert(data1 && data2);
    return *(int*)data1 - *(int*)data2;
}

#define LOOP 33

int main()
{
    int i;
    int* val;
    int* data;
    int* key;
    struct heap_t* heap;
    
    heap = heap_init(heap_cmp_func);
    assert(heap);

    data = (int*)malloc(sizeof(int) * LOOP);
    key = (int*)malloc(sizeof(int) * LOOP);
    for(i=0; i<LOOP; i++)
    {
        data[i] = rand() % (LOOP * 2);
        key[i] = heap_insert(heap, &data[i]);
        printf("%d ", data[i]);

        //_heap_debug(heap);
    }
    printf("\n");

    for(i = 0; i < LOOP; i++)
    {
        val = (int*)heap_erase(heap, key[i]);
        printf("%d ", *val);
        //_heap_debug(heap);
    }
/*
    while(heap_count(heap) > 0)
    {
        val = (int*)heap_pop(heap);
        printf("%d ", *val);
        _heap_debug(heap);
    }
    printf("\n");
*/
    free(data);
    free(key);
    heap_release(heap);

    getchar();
    return 0;
}

