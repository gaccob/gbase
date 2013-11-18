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
    for (i = 0; i<table->count; i++) {
        table->table[i].id = IDTS_INVALID_ID;
        table->table[i].ptr = NULL ;
    }
}

void idtable_clean_ex(idtable_t* table, idtable_free_func f)
{
    int32_t i;
    assert(table);
    for (i = 0; i<table->count; i++) {
        table->table[i].id = IDTS_INVALID_ID;
        f(table->table[i].ptr);
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

void idtable_release_ex(idtable_t* table, idtable_free_func f)
{
    if (table) {
        idtable_clean_ex(table, f);
        FREE(table->table);
        FREE(table);
    }
}
