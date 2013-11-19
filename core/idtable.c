#include <assert.h>
#include "idtable.h"

typedef struct idtable_item_t
{
    // <0 means not used
    int32_t id;
    void* ptr;
} idtable_item_t;

typedef struct idtable_t
{
    idtable_item_t* table;
    int32_t count;
} idtable_t;

#define IDTS_INVALID_ID -1

idtable_t* idtable_init(int32_t max_count)
{
    idtable_t* table;
    int32_t index;

    assert(max_count > 0);
    table = (idtable_t*)MALLOC(sizeof(idtable_t));
    assert(table);

    table->count = max_count;
    table->table = (idtable_item_t*)MALLOC(sizeof(idtable_item_t) * max_count);
    assert(table->table);
    for (index = 0; index < max_count; index ++) {
        table->table[index].id = IDTS_INVALID_ID;
        table->table[index].ptr = 0;
    }
    return table;
}

int32_t idtable_add(idtable_t* table, int32_t id, void* ptr)
{
    int32_t i, index;
    assert(table && id >= 0 && ptr);
    index = id % table->count;
    i = index;
    do {
        if (IDTS_INVALID_ID != table->table[i].id) {
            i ++;
            i %= table->count;
        } else {
            table->table[i].id = id;
            table->table[i].ptr = ptr;
            return 0;
        }
    } while(index != i);
    return -1;
}

void* idtable_get(idtable_t* table, int32_t id)
{
    int32_t i, index;
    assert(table && id >= 0);
    index = id % table->count;
    i = index;
    do {
        if (table->table[i].id == id) {
            return table->table[i].ptr;
        } else if (IDTS_INVALID_ID == table->table[i].id) {
            return NULL;
        }
        i ++;
        i %= table->count;
    } while(index != i);
    return NULL;
}

void idtable_remove(idtable_t* table, int32_t id)
{
    int32_t index, i;
    assert(table && id >= 0);
    index = id % table->count;
    i = index;
    do {
        if (IDTS_INVALID_ID == table->table[i].id) {
            return;
        }
        if (id == table->table[i].id) {
            table->table[i].id = IDTS_INVALID_ID;
            table->table[i].ptr = NULL;
            return;
        }
        i ++;
        i %= table->count;
    } while(index != i);
}

void idtable_cleanup(idtable_t* table)
{
    int32_t i;
    assert(table);
    for (i = 0; i < table->count; ++ i) {
        table->table[i].id = IDTS_INVALID_ID;
        table->table[i].ptr = NULL ;
    }
}

void idtable_release(idtable_t* table)
{
    if (table) {
        idtable_cleanup(table);
        FREE(table->table);
        FREE(table);
    }
}

typedef struct idtable_iterator_t {
    idtable_t* table;
    int32_t start_idx;
    int32_t end_idx;
} idtable_iterator_t;

idtable_iterator_t* idtable_iterator_init(idtable_t* table, int start_idx)
{
    if (table) {
        idtable_iterator_t* it = (idtable_iterator_t*)MALLOC(sizeof(*it));
        if (!it) return NULL;
        it->table = table;
        it->start_idx = start_idx % table->count;
        it->end_idx = it->start_idx;
        return it;
    }
    return NULL;
}

int32_t idtable_iterator_loop(idtable_iterator_t* it)
{
    while (it) {
        if (it->table->table[it->start_idx].id != IDTS_INVALID_ID) {
            it->start_idx = (it->start_idx + 1) % it->table->count;
            return 0;
        }
        it->start_idx = (it->start_idx + 1) % it->table->count;
        if (it->start_idx == it->end_idx) {
            return -1;
        }
    }
    return -1;
}

void* idtable_iterator_value(idtable_iterator_t* it)
{
    int32_t idx;
    if (it) {
        idx = (it->start_idx + it->table->count - 1) % it->table->count;
        return it->table->table[idx].ptr;
    }
    return NULL;
}

int32_t idtable_iterator_id(idtable_iterator_t* it)
{
    int32_t idx;
    if (it) {
        idx = (it->start_idx + it->table->count - 1) % it->table->count;
        return it->table->table[idx].id;
    }
    return -1;
}

void idtable_iterator_release(idtable_iterator_t* it)
{
    if (it) {
        FREE(it);
    }
}

