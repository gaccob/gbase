#include <assert.h>
#include "heap.h"

// minimum heap
typedef struct node_t {
    void* data;
    int heap_key;
} node_t;

typedef struct heap_t {
    node_t* array;
    size_t cap;
    size_t size;
    heap_cmp_func cmp_func;
    int next_key;
    int* key_table;
} heap_t;

#define HEAP_DEFAULT_SIZE 16

#define HEAP_PARENT(pos) ((pos - 1) >> 1)
#define HEAP_LEFT_CHILD(pos) (((pos) << 1) + 1)
#define HEAP_RIGHT_CHILD(pos) (((pos) << 1) + 2)

heap_t*
heap_create(heap_cmp_func cmp) {
    heap_t* heap = (heap_t*)MALLOC(sizeof(heap_t));
    if (!heap)
        goto HEAP_FAIL;
    heap->cap = HEAP_DEFAULT_SIZE;
    heap->array = (node_t*)MALLOC(sizeof(node_t) * heap->cap);
    if (!heap->array)
        goto HEAP_FAIL1;
    heap->size = 0;
    heap->cmp_func = cmp;
    heap->next_key = 0;
    heap->key_table = (int*)MALLOC(sizeof(int) * heap->cap);
    if (!heap->key_table)
        goto HEAP_FAIL2;
    for (size_t index = 0; index < heap->cap; index ++) {
        heap->array[index].data = 0;
        heap->array[index].heap_key = -1;
        heap->key_table[index] = -1;
    }
    return heap;

HEAP_FAIL2:
    FREE(heap->array);
HEAP_FAIL1:
    FREE(heap);
HEAP_FAIL:
    return NULL;
}

void
heap_release(heap_t* heap) {
    if (heap) {
        FREE(heap->key_table);
        FREE(heap->array);
        FREE(heap);
    }
}

static void
_heap_swap(heap_t* heap, int pos1, int pos2) {
    void* temp_data = heap->array[pos1].data;
    int temp_key = heap->array[pos1].heap_key;

    // update key table
    heap->key_table[heap->array[pos1].heap_key] = pos2;
    heap->key_table[heap->array[pos2].heap_key] = pos1;

    // swap array
    heap->array[pos1].data = heap->array[pos2].data;
    heap->array[pos1].heap_key= heap->array[pos2].heap_key;
    heap->array[pos2].data = temp_data;
    heap->array[pos2].heap_key = temp_key;
}

static int
_heap_full(heap_t* heap) {
    return (heap && heap->cap == heap->size) ? 0: -1;
}

static int
_heap_realloc(heap_t* heap) {
    size_t new_size = heap->cap * 2;
    node_t* new_array = (node_t*)MALLOC(sizeof(node_t) * new_size);
    if (!new_array)
        return -1;

    int* new_key_table = (int*)MALLOC(sizeof(int) * new_size);
    if (!new_key_table) {
        FREE(new_array);
        return -1;
    }

    for (size_t index = 0; index < heap->cap; index ++) {
        new_array[index].data = heap->array[index].data;
        new_array[index].heap_key = heap->array[index].heap_key;
        new_key_table[index] = heap->key_table[index];
    }
    for (size_t index = heap->cap; index < new_size; index ++) {
        new_array[index].data = 0;
        new_array[index].heap_key = -1;
        new_key_table[index] = -1;
    }
    FREE(heap->array);
    FREE(heap->key_table);

    heap->array = new_array;
    heap->key_table = new_key_table;
    heap->cap = new_size;
    return 0;
}

static void
_heap_set_next_key(heap_t* heap) {
    if (heap) {
        for (int key = (heap->next_key + 1) % heap->cap;
            key != heap->next_key;
            key = (key + 1) % heap->cap) {
            if (heap->key_table[key] < 0) {
                heap->next_key = key;
                return;
            }
        }
        // heap is full
        heap->next_key = heap->cap;
    }
}

static void
_heap_rotdown(heap_t* heap, int pos) {
    if (!heap)
        return;
    // rotate down
    while (pos < (int)heap->size) {
        int left = HEAP_LEFT_CHILD(pos);
        int right = HEAP_RIGHT_CHILD(pos);
        // no left & right
        if (left >= (int)heap->size) {
            break;
        }
        // no right, left < pos
        else if (right >= (int)heap->size) {
            if (heap->cmp_func(heap->array[left].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, left);
                pos = left;
            } else {
                break;
            }
        }
        // left < right, check to swap with left
        else if (heap->cmp_func(heap->array[left].data, heap->array[right].data) < 0) {
            if (heap->cmp_func(heap->array[left].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, left);
                pos = left;
            } else {
                break;
            }
        }
        // right <= left, check to swap with right
        else {
            if (heap->cmp_func(heap->array[right].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, right);
                pos = right;
            } else {
                break;
            }
        }
    }
}

//  return >= 0, success, return key which used to erase data
//  return < 0, fail
int
heap_insert(heap_t* heap, void* data) {
    if (!heap || !data) {
        return -1;
    }
    if (_heap_full(heap) == 0) {
        if (_heap_realloc(heap) < 0) {
            return -1;
        }
    }
    // insert data
    node_t* node = &heap->array[heap->size ++];
    node->data = data;
    node->heap_key = heap->next_key;
    int res = node->heap_key;

    // set key flag
    heap->key_table[node->heap_key] = heap->size - 1;
    _heap_set_next_key(heap);

    // rotate up
    int pos = heap->size - 1;
    while (pos > 0) {
        int pos_up = HEAP_PARENT(pos);
        if (heap->cmp_func(heap->array[pos].data, heap->array[pos_up].data) >= 0) {
            break;
        }
        _heap_swap(heap, pos, pos_up);
        pos = pos_up;
    }
    return res;
}

void*
heap_erase(heap_t* heap, int key) {
    if (!heap || heap->key_table[key] < 0) {
        return NULL;
    }
    int index = heap->key_table[key];
    void* data = heap->array[index].data;

    // set heap next heap key
    if (heap->next_key == (int)heap->cap) {
        heap->next_key = key;
    }

    _heap_swap(heap, index, heap->size - 1);
    heap->size --;

    _heap_rotdown(heap, index);

    heap->key_table[key] = -1;
    return data;
}

void
heap_update(heap_t* heap, int key, void* data) {
    if (!heap || !data)
        return;
    int index = heap->key_table[key];
    if (index < 0)
        return;
    heap->array[index].data = data;
    _heap_rotdown(heap, index);
}

int
heap_size(heap_t* heap) {
    return heap ? (int)heap->size : -1;
}

void*
heap_top(heap_t* heap) {
    if (heap && heap->size > 0) {
        return heap->array[0].data;
    }
    return NULL;
}

void*
heap_pop(heap_t* heap) {
    if (!heap || 0 == heap->size) {
        return NULL;
    }
    void* res = heap->array[0].data;
    // swap tail and head
    _heap_swap(heap, 0, heap->size - 1);
    heap->size --;
    _heap_rotdown(heap, 0);
    return res;
}

# if 0
static void
_heap_debug(heap_t* heap) {
    int i;
    printf("DEBUG: ");
    for (i = 0; i<heap->size; i++) {
        printf("%d[%d] ", *(int*)heap->array[i].data, heap->array[i].heap_key);
    }
    printf("\n");
}
#endif


