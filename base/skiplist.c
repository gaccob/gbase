#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "skiplist.h"

typedef struct skiplist_node_t {
    struct skiplist_node_t* next[MAX_SKIPLIST_LEVEL];
    struct skiplist_node_t* prev[MAX_SKIPLIST_LEVEL];
    int level;
    void* data;
} skiplist_node_t;

struct skiplist_t {
    skiplist_node_t head;
    skiplist_cmp_func cmp;
};

skiplist_t* skiplist_create(skiplist_cmp_func cmp) {
    skiplist_t* sl = (skiplist_t*)MALLOC(sizeof(skiplist_t));
    sl->cmp = cmp;
    memset(&sl->head, 0, sizeof(sl->head));
    return sl;
}

void
skiplist_release(skiplist_t* sl) {
    if (sl) {
        skiplist_node_t* prev = &sl->head;
        skiplist_node_t* next = NULL;
        while (prev) {
            next = prev->next[0];
            if (prev != &sl->head) {
                FREE(prev);
            }
            prev = next;
        }
        FREE(sl);
    }
}

int
_skiplist_rand_level() {
    int random = rand() % (1 << MAX_SKIPLIST_LEVEL) + 1;
    for (int i = 1; i <= MAX_SKIPLIST_LEVEL; ++ i) {
        if (random >= (1 << (MAX_SKIPLIST_LEVEL - i))) {
            return i - 1;
        }
    }
    assert(0);
    return -1;
}

void
_skiplist_insert_node(skiplist_node_t* prev, skiplist_node_t* node, int level) {
    assert(prev && node && level >= 0 && level < MAX_SKIPLIST_LEVEL);
    skiplist_node_t* next = prev->next[level];
    prev->next[level] = node;
    node->prev[level] = prev;
    node->next[level] = next;
    if (next) {
        next->prev[level] = node;
    }
}

void
_skiplist_erase_node(skiplist_node_t* node, int level) {
    assert(node && level >= 0 && level < MAX_SKIPLIST_LEVEL);
    skiplist_node_t* prev = node->prev[level];
    skiplist_node_t* next = node->next[level];
    if (prev) {
        prev->next[level] = next;
    }
    if (next) {
        next->prev[level] = prev;
    }
}

int
skiplist_insert(skiplist_t* sl, void* data) {
    if (!sl || !data) {
        return -1;
    }
    skiplist_node_t* node = (skiplist_node_t*)MALLOC(sizeof(skiplist_node_t));
    memset(node, 0, sizeof(skiplist_node_t));
    node->data = data;
    node->level = _skiplist_rand_level();
    printf("level=%d\n", node->level);

    // insert by level
    skiplist_node_t* prev = &sl->head;
    int level = node->level;
    while (level >= 0) {
        skiplist_node_t* next = prev->next[level];
        while (next && sl->cmp(next->data, node->data) <= 0) {
            prev = next;
            next = next->next[level];
        }
        _skiplist_insert_node(prev, node, level);
        -- level;
    }
    return 0;
}

void*
skiplist_find(skiplist_t* sl, void* data, int erase) {
    if (!sl || !data) {
        return NULL;
    }
    int level = MAX_SKIPLIST_LEVEL - 1;
    skiplist_node_t* prev = &sl->head;
    while (level >= 0) {
        skiplist_node_t* next = prev->next[level];
        if (next == NULL) {
            -- level;
            continue;
        }
        while (next) {
            int ret = sl->cmp(next->data, data);
            if (ret == 0) {
                if (erase == 0) {
                    while (level >= 0) {
                        _skiplist_erase_node(next, level --);
                    }
                }
                return next->data;
            } else if (ret < 0) {
                prev = next;
                next = next->next[level];
            } else {
                break;
            }
        }
        -- level;
    }
    return NULL;
}

void
skiplist_debug(skiplist_t* sl, skiplist_tostring_func tostring) {
    if (sl) {
        skiplist_node_t* from = &sl->head;
        for (int i = 0; i < MAX_SKIPLIST_LEVEL; ++ i) {
            skiplist_node_t* node = from->next[i];
            printf("level[%d]: ", i);
            while (node) {
                assert(node->data);
                printf("%s ", tostring(node->data));
                node = node->next[i];
            }
            printf("\n");
        }
    }
}
