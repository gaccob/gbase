#include <assert.h>
#include "idtable.h"

typedef struct node_t {
    // <0 means not used
    int id;
    void* ptr;
} node_t;

typedef struct idtable_t {
    node_t* table;
    int count;
} idtable_t;

#define IDTS_INVALID_ID -1

idtable_t*
idtable_create(int max_count) {
    assert(max_count > 0);
    idtable_t* table = (idtable_t*)MALLOC(sizeof(idtable_t));
    if (!table) {
        return NULL;
    }
    table->count = max_count;
    table->table = (node_t*)MALLOC(sizeof(node_t) * max_count);
    if (!table->table) {
        FREE(table);
        return NULL;
    }
    for (int index = 0; index < max_count; index ++) {
        table->table[index].id = IDTS_INVALID_ID;
        table->table[index].ptr = 0;
    }
    return table;
}

int
idtable_add(idtable_t* table, int id, void* ptr) {
    if (!table || id < 0 || !ptr)
        return -1;
    int index = id % table->count;
    int i = index;
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

void*
idtable_get(idtable_t* table, int id) {
    if (!table || id < 0)
        return NULL;
    int index = id % table->count;
    int i = index;
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

void
idtable_remove(idtable_t* table, int id) {
    if (!table || id < 0)
        return;
    int index = id % table->count;
    int i = index;
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

void
idtable_cleanup(idtable_t* table) {
    if (table) {
        for (int i = 0; i < table->count; ++ i) {
            table->table[i].id = IDTS_INVALID_ID;
            table->table[i].ptr = NULL ;
        }
    }
}

void
idtable_release(idtable_t* table) {
    if (table) {
        idtable_cleanup(table);
        FREE(table->table);
        FREE(table);
    }
}

int
idtable_loop(idtable_t* table, idtable_loop_func func, void* arg, int start) {
    if (table) {
        start = (start < 0 ? -start : start) % table->count;
        for (int i = start; i < table->count + start; ++ i) {
            if (table->table[i % table->count].id != IDTS_INVALID_ID) {
                int ret = func(table->table[i % table->count].ptr, arg);
                if (ret) return ret;
            }
        }
    }
    return 0;
}

