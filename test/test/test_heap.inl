#include "base/heap.h"

int
heap_cmp(void* data1, void* data2) {
    assert(data1 && data2);
    return *(int*)data1 - *(int*)data2;
}

#define HEAP_LOOP 33

int
test_heap() {
    int i;
    int* val;
    int* data;
    int* key;
    struct heap_t* heap;

    heap = heap_create(heap_cmp);
    assert(heap);

    data = (int*)MALLOC(sizeof(int) * HEAP_LOOP);
    key = (int*)MALLOC(sizeof(int) * HEAP_LOOP);
    for (i=0; i<HEAP_LOOP; i++) {
        data[i] = rand() % (HEAP_LOOP * 2);
        key[i] = heap_insert(heap, &data[i]);
        printf("%d ", data[i]);
        //_heap_debug(heap);
    }
    printf("\n");
/*
    for (i = 0; i < HEAP_LOOP; i++) {
        val = (int*)heap_erase(heap, key[i]);
        printf("%d ", *val);
        //_heap_debug(heap);
    }
*/
    while (heap_size(heap) > 0) {
        val = (int*)heap_pop(heap);
        printf("%d ", *val);
        // _heap_debug(heap);
    }
    printf("\n");

    FREE(data);
    FREE(key);
    heap_release(heap);
    return 0;
}

