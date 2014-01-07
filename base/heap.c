#include <assert.h>
#include "heap.h"

// minimum heap
typedef struct heap_node_t
{
    void* data;
    int heap_key;
} heap_node_t;

typedef struct heap_t
{
    heap_node_t* array;
    size_t size;
    size_t count;
    heap_cmp cmp_func;
    int next_key;
    int* key_table;
} heap_t;

#define HEAP_DEFAULT_SIZE 16

#define HEAP_PARENT(pos) ((pos - 1) >> 1)
#define HEAP_LEFT_CHILD(pos) (((pos) << 1) + 1)
#define HEAP_RIGHT_CHILD(pos) (((pos) << 1) + 2)

heap_t* heap_init(heap_cmp cmp)
{
    size_t index;
    heap_t* heap;

    heap = (heap_t*)MALLOC(sizeof(heap_t));
    if (!heap) goto HEAP_FAIL;

    heap->size = HEAP_DEFAULT_SIZE;
    heap->array = (heap_node_t*)MALLOC(sizeof(heap_node_t) * heap->size);
    if (!heap->array) goto HEAP_FAIL1;

    heap->count = 0;
    heap->cmp_func = cmp;
    heap->next_key = 0;

    heap->key_table = (int*)MALLOC(sizeof(int) * heap->size);
    if (!heap->key_table) goto HEAP_FAIL2;

    for (index = 0; index < heap->size; index ++) {
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

void heap_release(heap_t* heap)
{
    if (heap) {
        FREE(heap->key_table);
        FREE(heap->array);
        FREE(heap);
    }
}

void _heap_swap(heap_t* heap, int pos1, int pos2)
{
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

int _heap_full(heap_t* heap)
{
    return (heap && heap->size == heap->count) ? 0: -1;
}

int _heap_realloc(heap_t* heap)
{
    size_t index, new_size;
    heap_node_t* new_array;
    int* new_key_table;

    new_size = heap->size * 2;
    new_array = (heap_node_t*)MALLOC(sizeof(heap_node_t) * new_size);
    if (!new_array) return -1;

    new_key_table = (int*)MALLOC(sizeof(int) * new_size);
    if (!new_key_table) {
        FREE(new_array);
        return -1;
    }

    for (index = 0; index < heap->size; index ++) {
        new_array[index].data = heap->array[index].data;
        new_array[index].heap_key = heap->array[index].heap_key;
        new_key_table[index] = heap->key_table[index];
    }
    for(index = heap->size; index < new_size; index ++) {
        new_array[index].data = 0;
        new_array[index].heap_key = -1;
        new_key_table[index] = -1;
    }
    FREE(heap->array);
    FREE(heap->key_table);

    heap->array = new_array;
    heap->key_table = new_key_table;
    heap->size = new_size;
    return 0;
}

void _heap_set_next_key(heap_t* heap)
{
    int key;
    assert(heap);
    for (key = (heap->next_key + 1) % heap->size;
        key != heap->next_key;
        key = (key + 1) % heap->size) {
        if (heap->key_table[key] < 0) {
            heap->next_key = key;
            return;
        }
    }

    // heap is full
    heap->next_key = heap->size;
}

void _heap_rotdown(heap_t* heap, int pos)
{
    int pos_left, pos_right;
    if (!heap) return;

    // rotate down
    while (pos < (int)heap->count) {
        pos_left = HEAP_LEFT_CHILD(pos);
        pos_right = HEAP_RIGHT_CHILD(pos);
        // no left & right
        if (pos_left >= (int)heap->count) {
            break;
        }
        // no right, left < pos
        else if (pos_right >= (int)heap->count) {
            if (heap->cmp_func(heap->array[pos_left].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, pos_left);
                pos = pos_left;
            } else {
                break;
            }
        }
        // left < right, check to swap with left
        else if (heap->cmp_func(heap->array[pos_left].data, heap->array[pos_right].data) < 0) {
            if (heap->cmp_func(heap->array[pos_left].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, pos_left);
                pos = pos_left;
            } else {
                break;
            }
        }
        // right <= left, check to swap with right
        else {
            if (heap->cmp_func(heap->array[pos_right].data, heap->array[pos].data) < 0) {
                _heap_swap(heap, pos, pos_right);
                pos = pos_right;
            } else {
                break;
            }
        }
    }
}

//  return >= 0, success, return key which used to erase data
//  return < 0, fail
int heap_insert(heap_t* heap, void* data)
{
    int pos, pos_up, res;
    heap_node_t* node;

    if (!heap || !data) return -1;

    if (0 == _heap_full(heap)) {
        if (_heap_realloc(heap) < 0) {
            return -1;
        }
    }

    // insert data
    node = &heap->array[heap->count ++];
    node->data = data;
    node->heap_key = heap->next_key;
    res = node->heap_key;

    // set key flag
    heap->key_table[node->heap_key] = heap->count - 1;
    _heap_set_next_key(heap);

    // rotate up
    pos = heap->count - 1;
    while (pos > 0) {
        pos_up = HEAP_PARENT(pos);
        if (heap->cmp_func(heap->array[pos].data, heap->array[pos_up].data) >= 0) {
            break;
        }
        _heap_swap(heap, pos, pos_up);
        pos = pos_up;
    }
    return res;
}

void* heap_erase(heap_t* heap, int key)
{
    int index;
    void* data;
    if (!heap || heap->key_table[key] < 0) {
        return NULL;
    }

    // get key data
    index = heap->key_table[key];
    data = heap->array[index].data;

    // set heap next heap key
    if (heap->next_key == (int)heap->size) {
        heap->next_key = key;
    }

    // swap index and head
    _heap_swap(heap, index, heap->count - 1);
    heap->count --;

    // rotate down
    _heap_rotdown(heap, index);

    // reset key flag
    heap->key_table[key] = -1;
    return data;
}

void heap_update(heap_t* heap, int key, void* data)
{
    int index;
    if (!heap || !data) return;

    // get key
    index = heap->key_table[key];
    if (index < 0) return;
    heap->array[index].data = data;
    // rotate down
    _heap_rotdown(heap, index);
}

int heap_count(heap_t* heap)
{
    if (!heap) return -1;
    return (int)heap->count;
}

void* heap_top(heap_t* heap)
{
    if (heap && heap->count > 0) {
        return heap->array[0].data;
    }
    return NULL;
}

void* heap_pop(heap_t* heap)
{
    void* res;
    if (!heap || 0 == heap->count) {
        return NULL;
    }
    res = heap->array[0].data;

    // swap tail and head
    _heap_swap(heap, 0, heap->count - 1);
    heap->count --;

    // rotate down
    _heap_rotdown(heap, 0);
    return res;
}

#if 0
void _heap_debug(heap_t* heap)
{
    int i;
    printf("DEBUG: ");
    for(i = 0; i<heap->count; i++)
        printf("%d[%d] ", *(int*)heap->array[i].data, heap->array[i].heap_key);
    printf("\n");
}
#endif

