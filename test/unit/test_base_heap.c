#include <assert.h>
#include "base/heap.h"
#include "util/random.h"

static int
_heap_cmp(void* data1, void* data2) {
    assert(data1 && data2);
    return *(int*)data1 - *(int*)data2;
}

static int _size = 33;

int
test_base_heap(char* param) {
    if (param) {
        _size = atoi(param);
    }
    int _max = (_size << 2);

    heap_t* heap = heap_create(_heap_cmp);
    if (!heap) {
        fprintf(stderr, "heap create fail\n");
        return -1;
    }

    rand_seed((uint32_t)time(NULL));
    int* data = (int*)MALLOC(sizeof(int) * _size);
    int* key = (int*)MALLOC(sizeof(int) * _size);
    for (int i = 0; i < _size; ++ i) {
        data[i] = rand() % _max;
    }

    // insert
    for (int i = 0; i < _size; ++ i) {
        key[i] = heap_insert(heap, &data[i]);
        if (key[i] < 0) {
            fprintf(stderr, "heap insert fail: %d\n", key[i]);
            heap_release(heap);
            return -1;
        }
    }
    if (heap_size(heap) != _size) {
        fprintf(stderr, "head size fail!\n");
        heap_release(heap);
        return -1;
    }
    printf("heap insert success\n");

    // erase
    for (int i = 0; i < _size; ++ i) {
        int* val = (int*)heap_erase(heap, key[i]);
        if (*val != data[i]) {
            fprintf(stderr, "head erase fail!\n");
            heap_release(heap);
            return -1;
        }
    }
    printf("heap erase success\n");

    if (heap_size(heap) != 0) {
        fprintf(stderr, "head size fail!\n");
        heap_release(heap);
        return -1;
    }
    printf("heap size success\n");

    // insert
    for (int i = 0; i < _size; ++ i) {
        key[i] = heap_insert(heap, &data[i]);
        if (key[i] < 0) {
            fprintf(stderr, "heap insert fail: %d\n", key[i]);
            heap_release(heap);
            return -1;
        }
    }

    // pop
    int top_1, top_2 = -1;
    while (heap_size(heap) > 0) {
        int* val = (int*)heap_pop(heap);
        top_1 = top_2;
        top_2 = *val;
        if (top_1 > top_2) {
            fprintf(stderr, "heap pop order fail\n");
            heap_release(heap);
            return -1;
        }
    }
    printf("heap pop success\n");

    FREE(data);
    FREE(key);
    heap_release(heap);
    return 0;
}

