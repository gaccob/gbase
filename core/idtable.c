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

struct idtable_t* idtable_init(int32_t max_count)
{
    struct idtable_t* table;
    int32_t index;

    assert(max_count > 0);
    table = (struct idtable_t*)MALLOC(sizeof(struct idtable_t));
    assert(table);

    table->count = max_count;
    table->table = (struct idtable_item_t*)MALLOC(sizeof(struct idtable_item_t) * max_count);
    assert(table->table);
    for (index = 0; index < max_count; index ++) {
        table->table[index].id = IDTS_INVALID_ID;
        table->table[index].ptr = 0;
    }
    return table;
}

int32_t idtable_add(struct idtable_t* table, int32_t id, void* ptr)
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

void* idtable_get(struct idtable_t* table, int32_t id)
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

void idtable_remove(struct idtable_t* table, int32_t id)
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

void idtable_cleanup(struct idtable_t* table)
{
    int32_t i;
    assert(table);
    for (i = 0; i<table->count; i++) {
        table->table[i].id = IDTS_INVALID_ID;
        table->table[i].ptr = NULL ;
    }
}

void idtable_release(struct idtable_t* table)
{
    if (table) {
        FREE(table->table);
        FREE(table);
    }
}



