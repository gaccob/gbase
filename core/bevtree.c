#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "core/bevtree.h"
#include "core/os_def.h"

enum BVTNodeType {
    BVT_NODE_SELECTOR,
    BVT_NODE_SEQUENCE,
    BVT_NODE_PARALLEL,
    BVT_NODE_CONDITION,
    BVT_NODE_ACTION,
};

enum BVTParallelType {
    BVT_PARALLEL_ALL, // all ok, then ok
    BVT_PARALLEL_ONE, // one ok, then ok
};

enum BVTSelectorType {
    BVT_SELECTOR_COND, // condition selector
    BVT_SELECTOR_WEIGHT, // selector by weight
};

#define BVT_DEFAULT_TABLE_SIZE 1024
typedef struct bvt_cbtable_t {
    size_t size;
    bvt_func* table;
} bvt_cbtable_t;

#define BVT_MAX_NAME_LEN 128
typedef struct bvt_t {
    char name[BVT_MAX_NAME_LEN];
    enum BVTNodeType type;
    int32_t weight;
    union {
        struct {
            int32_t callback_id;
        } act_args;
        struct {
            int32_t callback_id;
        } con_args;
        struct {
            enum BVTParallelType type;
        } par_args;
        struct {
            enum BVTSelectorType type;
        } sel_args;
    };
    struct bvt_t* next;
    struct bvt_t* condition;
    struct bvt_t* child;
    bvt_cbtable_t* cb_table;
} bvt_t;

#ifdef BVT_DEBUG
    #define BVT_DEBUG_LOG(node) \
        do { \
            switch (node->type) { \
                case BVT_NODE_SELECTOR: \
                    printf("selector [%s]\n", node->name); \
                    break; \
                case BVT_NODE_SEQUENCE: \
                    printf("sequence [%s]\n", node->name); \
                    break; \
                case BVT_NODE_PARALLEL: \
                    printf("parallel [%s]\n", node->name); \
                    break; \
                case BVT_NODE_CONDITION: \
                    printf("condition [%s]\n", node->name); \
                    break; \
                case BVT_NODE_ACTION: \
                    printf("action [%s]\n", node->name); \
                    break; \
                default: \
                    printf("invalid type [%s]\n", node->name); \
                    break; \
            } \
        } while (0)
#else
    #define BVT_DEBUG_LOG(node) (void)(node)
#endif

static int32_t _bvt_run(bvt_t* n, bvt_cbtable_t* t, void* input);

static int32_t
_bvt_run_select(bvt_t* n, bvt_cbtable_t* t, void* input) {
    bvt_t* c = NULL;
    int32_t ret, sum, res;

    BVT_DEBUG_LOG(n);
    // condition node
    c = n->condition;
    while (c) {
        ret = _bvt_run(c, t, input);
        if (ret != BVT_SUCCESS)
            return BVT_BACKTRACK;
        c = c->next;
    }

    // children
    c = n->child;
    if (!c) return BVT_ERROR;

    // condition selector
    if (n->sel_args.type == BVT_SELECTOR_COND) {
        while (c) {
            ret = _bvt_run(c, t, input);
            // condition check fail, go next
            if (ret == BVT_BACKTRACK) {
                c = c->next;
                continue;
            }
            return ret;
        }
    }
    // weight selector
    else if (n->sel_args.type == BVT_SELECTOR_WEIGHT) {
        sum = 0;
        c = n->child;
        while (c) {
            sum += c->weight;
            c = c->next;
        }
        res = rand() % sum;
        c = n->child;
        while (c) {
            if (res > c->weight) {
                res -= c->weight;
                c = c->next;
                continue;
            }
            return _bvt_run(c, t, input);
        }
    }
    return BVT_ERROR;
}

static int32_t
_bvt_run_condition(bvt_t* n, bvt_cbtable_t* t, void* input) {
    bvt_func check;
    BVT_DEBUG_LOG(n);
    if (n->con_args.callback_id < 0 ||
        n->con_args.callback_id >= (int)t->size)
        return BVT_CONDITION_ERROR;

    check = t->table[n->con_args.callback_id];
    return (*check)(input);
}

static int32_t
_bvt_run_sequence(bvt_t* n, bvt_cbtable_t* t, void* input) {
    bvt_t* c = NULL;
    int32_t ret;

    BVT_DEBUG_LOG(n);
    c = n->condition;
    while (c) {
        if (_bvt_run(c, t, input) != BVT_SUCCESS)
            return BVT_BACKTRACK;
        c = c->next;
    }

    c = n->child;
    if (!c) return BVT_SEQUENCE_ERROR;
    while (c) {
        ret = _bvt_run(c, t, input);
        if (ret != BVT_SUCCESS)
            return ret;
        c = c->next;
    };
    return BVT_SUCCESS;
}

static int32_t
_bvt_run_parallel(bvt_t* n, bvt_cbtable_t* t, void* input) {
    bvt_t* c = NULL;
    int32_t ret, cret;

    BVT_DEBUG_LOG(n);
    c = n->condition;
    while (c) {
        if (_bvt_run(c, t, input) != BVT_SUCCESS)
            return BVT_BACKTRACK;
        c = c->next;
    }

    c = n->child;
    if (!c) return BVT_PARALLEL_ERROR;

    ret = BVT_ERROR;
    while (c) {
        cret = _bvt_run(c, t, input);
        if (cret != BVT_SUCCESS) {
            switch (n->par_args.type) {
                case BVT_PARALLEL_ALL:
                    return cret;
                case BVT_PARALLEL_ONE:
                    break;
                default:
                    return BVT_PARALLEL_ERROR;
            }
        } else {
            ret = BVT_SUCCESS;
        }
        c = c->next;
    }
    return ret;
}

static int32_t
_bvt_run_action(bvt_t* n, bvt_cbtable_t* t, void* input) {
    bvt_func action;
    int32_t ret;

    BVT_DEBUG_LOG(n);
    if (n->act_args.callback_id < 0 ||
        n->act_args.callback_id >= (int)t->size) {
        return BVT_ACTION_ERROR;
    }
    action = t->table[n->act_args.callback_id];
    ret = (*action)(input);
    if (ret == BVT_SUCCESS) {
        printf("action [%s] success\n", n->name);
    }
    return ret;
}

static void
_bvt_release_node(bvt_t* n) {
    if (n) {
        if (n->child) {
            bvt_t* c = n->child;
            while (c) {
                bvt_t* next = c->next;
                _bvt_release_node(c);
                c = next;
            }
        }
        if (n->condition) {
            bvt_t* c = n->condition;
            while (c) {
                bvt_t* next = c->next;
                _bvt_release_node(c);
                c = next;
            }
        }
        FREE(n);
    }
}

void
bvt_release(bvt_t* n) {
    if (n) {
        if (n->cb_table) {
            if (n->cb_table->table)
                FREE(n->cb_table->table);
            FREE(n->cb_table);
            n->cb_table = NULL;
        }
        _bvt_release_node(n);
    }
}

static void
_bvt_debug(bvt_t* b, int32_t indent) {
    bvt_t* c = NULL;
    int32_t i = indent;
    while (i --) {
        if (i == 2) printf("|");
        else if (i < 2) printf("-");
        else printf(" ");
    }
    switch (b->type) {
        case BVT_NODE_CONDITION:
            printf("condition: %s %d\n", b->name, b->con_args.callback_id);
            break;
        case BVT_NODE_SELECTOR:
            printf("selector: %s\n", b->name);
            break;
        case BVT_NODE_SEQUENCE:
            printf("sequence: %s\n", b->name);
            break;
        case BVT_NODE_ACTION:
            printf("action: %s %d\n", b->name, b->act_args.callback_id);
            break;
        case BVT_NODE_PARALLEL:
            printf("parallel: %s\n", b->name);
            break;
    }
    c = b->condition;
    while (c) {
        _bvt_debug(c, indent + 3);
        c = c->next;
    }
    c = b->child;
    while (c) {
        _bvt_debug(c, indent + 3);
        c = c->next;
    }
}

void
bvt_debug(bvt_t* b) {
    if (b) {
        printf("\n================\n");
        _bvt_debug(b, 0);
        printf("\n\n");
    }
}

int32_t
bvt_register_callback(bvt_t* n, bvt_func cb, int32_t id) {
    if (!n || !cb || id < 0)
        return BVT_ERROR;

    if (!n->cb_table) {
        n->cb_table = (bvt_cbtable_t*)MALLOC(sizeof(bvt_cbtable_t));
        memset(n->cb_table, 0, sizeof(bvt_cbtable_t));
        n->cb_table->size = BVT_DEFAULT_TABLE_SIZE > id ? BVT_DEFAULT_TABLE_SIZE : (id + 1);
        n->cb_table->table = (bvt_func*)MALLOC(sizeof(bvt_func) * n->cb_table->size);
        memset(n->cb_table->table, 0, sizeof(bvt_func) * n->cb_table->size);
    } else if ((int)n->cb_table->size <= id) {
        bvt_func* c = n->cb_table->table;
        size_t oldsize = n->cb_table->size;
        while ((int)n->cb_table->size <= id) {
            n->cb_table->size *= 2;
        }
        n->cb_table->table = (bvt_func*)MALLOC(sizeof(bvt_func) * n->cb_table->size);
        memset(n->cb_table->table, 0, sizeof(bvt_func) * n->cb_table->size);
        memcpy(n->cb_table->table, c, sizeof(bvt_func) * oldsize);
    }

    if (n->cb_table->table[id]) {
        return BVT_CALLBACK_DUPLICATED;
    }
    n->cb_table->table[id] = cb;
    return BVT_SUCCESS;
}

static int32_t
_bvt_run(bvt_t* n, bvt_cbtable_t* t, void* input) {
    if (n) {
        switch (n->type) {
            case BVT_NODE_SELECTOR:
                return _bvt_run_select(n, t, input);
            case BVT_NODE_SEQUENCE:
                return _bvt_run_sequence(n, t, input);
            case BVT_NODE_PARALLEL:
                return _bvt_run_parallel(n, t, input);
            case BVT_NODE_CONDITION:
                return _bvt_run_condition(n, t, input);
            case BVT_NODE_ACTION:
                return _bvt_run_action(n, t, input);
        }
    }
    return BVT_ERROR;
}

int32_t
bvt_run(bvt_t* n, void* input) {
    if (n) {
        return _bvt_run(n, n->cb_table, input);
    }
    return BVT_ERROR;
}

// add gliffy extension
#include "bevtree_gliffy.ext"

