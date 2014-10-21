#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "skiplist.h"

typedef struct skiplist_node_t {
    struct {
        struct skiplist_node_t* next;
        struct skiplist_node_t* prev;
        int span;
    } link[MAX_SKIPLIST_LEVEL];
    int level;
    void* data;
} skiplist_node_t;

struct skiplist_t {
    int random[MAX_SKIPLIST_LEVEL];
    skiplist_node_t head;
    skiplist_cmp_func cmp;
};

skiplist_t* skiplist_create(skiplist_cmp_func cmp) {
    skiplist_t* sl = (skiplist_t*)malloc(sizeof(skiplist_t));
    sl->cmp = cmp;
    memset(&sl->head, 0, sizeof(sl->head));
    for (int i = 0; i < MAX_SKIPLIST_LEVEL; ++ i) {
        sl->random[i] = pow(10, i);
    }
    return sl;
}

void
skiplist_release(skiplist_t* sl) {
    if (sl) {
        skiplist_node_t* prev = &sl->head;
        skiplist_node_t* next = NULL;
        while (prev) {
            next = prev->link[0].next;
            if (prev != &sl->head) {
                free(prev);
            }
            prev = next;
        }
        free(sl);
    }
}

int
_skiplist_rand_level(skiplist_t* sl) {
    int random = rand() % sl->random[MAX_SKIPLIST_LEVEL - 1];
    for (int i = 0; i < MAX_SKIPLIST_LEVEL; ++ i) {
        if (random < sl->random[i]) {
            return MAX_SKIPLIST_LEVEL - 1 - i;
        }
    }
    assert(0);
    return -1;
}

void
_skiplist_insert_node(skiplist_node_t* prev, skiplist_node_t* node, int level) {
    assert(prev && node && level >= 0 && level < MAX_SKIPLIST_LEVEL);
    skiplist_node_t* next = prev->link[level].next;
    prev->link[level].next = node;
    node->link[level].prev = prev;
    node->link[level].next = next;
    if (next) {
        next->link[level].prev = node;
    }
}

void
_skiplist_erase_node(skiplist_node_t* node, int level) {
    assert(node && level >= 0 && level < MAX_SKIPLIST_LEVEL);
    skiplist_node_t* prev = node->link[level].prev;
    skiplist_node_t* next = node->link[level].next;
    if (prev) {
        prev->link[level].next = next;
    }
    if (next) {
        next->link[level].prev = prev;
        next->link[level].span += (node->link[level].span - 1);
    }
}

int
skiplist_insert(skiplist_t* sl, void* data) {
    if (!sl || !data) {
        return -1;
    }
    skiplist_node_t* node = (skiplist_node_t*)malloc(sizeof(skiplist_node_t));
    memset(node, 0, sizeof(skiplist_node_t));
    node->data = data;
    node->level = _skiplist_rand_level(sl);

    // insert by level
    skiplist_node_t* prev = &sl->head;
    int level = MAX_SKIPLIST_LEVEL - 1;
    int span[MAX_SKIPLIST_LEVEL];
    memset(span, 0, sizeof(span));
    while (level >= 0) {
        skiplist_node_t* next = prev->link[level].next;
        while (next && sl->cmp(next->data, node->data) <= 0) {
            span[level] += next->link[level].span;
            prev = next;
            next = next->link[level].next;
        }
        if (level <= node->level) {
            _skiplist_insert_node(prev, node, level);
        } else if (next) {
            ++ next->link[level].span;
        }
        -- level;
    }

    node->link[0].span = 1;
    for (int i = 1; i <= node->level; ++ i) {
        node->link[i].span = node->link[i - 1].span + span[i - 1];
        skiplist_node_t* next = node->link[i].next;
        if (next) {
            next->link[i].span -= (span[i - 1] - 1);
        }
    }
    return 0;
}

void*
skiplist_find(skiplist_t* sl, void* data, int* index, int erase) {
    if (!sl || !data) {
        return NULL;
    }
    int level = MAX_SKIPLIST_LEVEL - 1;
    skiplist_node_t* prev = &sl->head;
    skiplist_node_t* change[MAX_SKIPLIST_LEVEL];
    memset(change, 0, sizeof(change));
    int span = 0;
    while (level >= 0) {
        skiplist_node_t* next = prev->link[level].next;
        if (next == NULL) {
            -- level;
            continue;
        }
        while (next) {
            int ret = sl->cmp(next->data, data);
            if (ret == 0) {
                span += next->link[level].span;
                void* ret = next->data;
                // do erase
                if (erase == 0) {
                    while (level >= 0) {
                        _skiplist_erase_node(next, level --);
                    }
                    free(next);
                    // change upper next span
                    for (int i = 0; i < MAX_SKIPLIST_LEVEL; ++ i) {
                        if (change[i]) {
                            -- change[i]->link[i].span;
                        }
                    }
                }
                // index accumulated by span
                if (index) {
                   *index = span;
                }
                return ret;
            } else if (ret < 0) {
                span += next->link[level].span;
                prev = next;
                next = next->link[level].next;
            } else {
                change[level] = next;
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
            skiplist_node_t* node = from->link[i].next;
            printf("level[%d]: ", i);
            while (node) {
                assert(node->data);
                printf("%s[%d] ", tostring(node->data), node->link[i].span);
                node = node->link[i].next;
            }
            printf("\n");
        }
        printf("\n");
    }
}
