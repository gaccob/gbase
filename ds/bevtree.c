#include "bevtree.h"

#include "bevtree.h"

enum BVTStatus
{
    BVT_STATUS_SUCCESS,
    BVT_STATUS_FAIL,
    BVT_STATUS_RUNNING,
};

enum bvt_tType
{
    BVT_NODE_SELECTOR,
    BVT_NODE_SEQUENCE,
    BVT_NODE_PARALLEL,
    BVT_NODE_CONDITION,
    BVT_NODE_ACTION,
};

enum BVTSelectorType
{
    BVT_SELECTOR_NORMAL,
    BVT_SELECTOR_PRIORITY,
};

enum BVTParallelType
{
    BVT_PARALLEL_ALL, // all ok, then ok
    BVT_PARALLEL_ONE, // one ok, then ok
};

#define MAX_BVT_NAME_LEN 64

typedef struct bvt_callback_table
{
    size_t size;
    bvt_callback* table;
} bvt_callback_table;

typedef struct bvt_t
{
    char name[MAX_BVT_NAME_LEN];
    enum BVTStatus status;
    enum bvt_tType type;

    union {
        struct {
            enum BVTSelectorType type;
            struct bvt_t* last_sel_child;
        } sel_args;

        struct {
            int32_t callback_id;
        } act_args;

        struct {
            int32_t callback_id;
        } con_args;

        struct {
            enum BVTParallelType type;
        } par_args;
    };

    struct bvt_t* parent;    // parent = NULL means root
    struct bvt_t* next_sibling;
    struct bvt_t* prev_sibling;
    struct bvt_t* first_child;

    struct bvt_callback_table* cb_table;
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

int32_t bvt_run_select(struct bvt_t* n, void* input)
{
    if (!n || n->type != BVT_NODE_SELECTOR)
        return BVT_ERROR;

    BVT_DEBUG_LOG(n);

    // start select node
    bvt_t* start = 0;
    if (n->sel_args.type == BVT_SELECTOR_NORMAL) {
        start = (n->sel_args.last_sel_child != NULL
              ? n->sel_args.last_sel_child
              : n->first_child);
    } else if (n->sel_args.type == BVT_SELECTOR_PRIORITY) {
        start = n->first_child;
    } else {
        return BVT_SELECTOR_ERROR;
    }
    if (!start) {
        return BVT_SELECTOR_ERROR;
    }

    // child must has condition
    bvt_t* c = start;
    bvt_t* sel = 0;
    do {
        // first success execute child
        if (BVT_SUCCESS == bvt_run(c, input)) {
            sel = c;
            break;
        }
        c = c->next_sibling;
    } while (c != start);

    return BVT_SUCCESS;
}

int32_t bvt_run_condition(bvt_t* n, void* input)
{
    if (!n || n->type != BVT_NODE_CONDITION)
        return BVT_ERROR;

    BVT_DEBUG_LOG(n);

    if (n->con_args.callback_id < 0 ||
        n->con_args.callback_id >= n->cb_table->size)
        return BVT_CONDITION_ERROR;

    bvt_callback check = n->cb_table->table[n->con_args.callback_id];
    return (*check)(input);
}

int32_t bvt_run_sequence(bvt_t* n, void* input)
{
    if (!n || n->type != BVT_NODE_SEQUENCE)
        return BVT_ERROR;

    BVT_DEBUG_LOG(n);

    bvt_t* start = n->first_child;
    if (!start)
        return BVT_SEQUENCE_ERROR;

    // run all children by sequence
    bvt_t* c = start;
    do {
        int32_t ret = bvt_run(c, input);
        if (ret != BVT_SUCCESS)
            return ret;
        c = c->next_sibling;
    } while (start != c);

    return BVT_SUCCESS;
}

int32_t bvt_run_parallel(bvt_t* n, void* input)
{
    if (!n || n->type != BVT_NODE_PARALLEL)
        return BVT_ERROR;

    BVT_DEBUG_LOG(n);

    bvt_t* start = n->first_child;
    if (!start)
        return BVT_PARALLEL_ERROR;

    bvt_t* c = start;
    int32_t ret = BVT_ERROR;
    do {
        int32_t cret = bvt_run(c, input);
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
            ret = cret;
        }
        c = c->next_sibling;
    } while (c != start);
    return ret;
}

int32_t bvt_run_action(bvt_t* n, void* input)
{
    if (!n || n->type != BVT_NODE_ACTION)
        return BVT_ERROR;

    BVT_DEBUG_LOG(n);

    if (n->act_args.callback_id < 0 ||
        n->act_args.callback_id >= n->cb_table->size)
        return BVT_CONDITION_ERROR;

    bvt_callback action = n->cb_table->table[n->act_args.callback_id];
    return (*action)(input);
}

// json config file
struct bvt_t* bvt_init(const char* cfg)
{
    // TODO:
    return NULL;
}

int32_t bvt_register_callback(struct bvt_t* n, bvt_callback cb, int32_t id)
{
    if (!n || !cb || id < 0)
        return BVT_ERROR;

    if (!n->cb_table)
    {
        n->cb_table = (bvt_callback_table*)malloc(sizeof(bvt_callback_table));
        memset(n->cb_table, 0, sizeof(bvt_callback_table));
    }
}

int32_t bvt_run(struct bvt_t* n, void* input)
{
    if (n) {
        switch (n->type) {
            case BVT_NODE_SELECTOR:
                return bvt_run_select(n, input);
            case BVT_NODE_SEQUENCE:
                return bvt_run_sequence(n, input);
            case BVT_NODE_PARALLEL:
                return bvt_run_parallel(n, input);
            case BVT_NODE_CONDITION:
                return bvt_run_condition(n, input);
            case BVT_NODE_ACTION:
                return bvt_run_action(n, input);
        }
    }
    return BVT_ERROR;
}

void bvt_release(bvt_t* n)
{
    // TODO:
}

