#include <fcntl.h>
#include <unistd.h>
#include "ds/bevtree.h"
#include "core/cjson.h"

enum bvt_tType
{
    BVT_NODE_SELECTOR,
    BVT_NODE_SEQUENCE,
    BVT_NODE_PARALLEL,
    BVT_NODE_CONDITION,
    BVT_NODE_ACTION,
};

enum BVTParallelType
{
    BVT_PARALLEL_ALL, // all ok, then ok
    BVT_PARALLEL_ONE, // one ok, then ok
};

#define BVT_DEFAULT_TABLE_SIZE 1024
typedef struct bvt_callback_table
{
    size_t size;
    bvt_callback* table;
} bvt_callback_table;

#define BVT_MAX_NAME_LEN 64
typedef struct bvt_t
{
    char name[BVT_MAX_NAME_LEN];
    enum bvt_tType type;

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
    };

    struct bvt_t* next;
    struct bvt_t* condition;
    struct bvt_t* child;

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

int32_t _bvt_run(bvt_t* n, bvt_callback_table* t, void* input) ;

int32_t _bvt_run_select(bvt_t* n, bvt_callback_table* t, void* input)
{
    BVT_DEBUG_LOG(n);

    // condition node
    bvt_t* c = n->condition;
    while (c) {
        int32_t ret = _bvt_run(c, t, input);
        if (ret != BVT_SUCCESS)
            return ret;
        c = c->next;
    }

    // children
    c = n->child;
    if (!c)
        return BVT_ERROR;
    while (c) {
        int32_t ret = _bvt_run(c, t, input);
        // condition check fail, go next
        if (ret == BVT_BACKTRACK) {
            c = c->next;
            continue;
        }
        return ret;
    }
    return BVT_ERROR;
}

int32_t _bvt_run_condition(bvt_t* n, bvt_callback_table* t, void* input)
{
    BVT_DEBUG_LOG(n);
    if (n->con_args.callback_id < 0 ||
        n->con_args.callback_id >= t->size)
        return BVT_CONDITION_ERROR;

    bvt_callback check = t->table[n->con_args.callback_id];
    return (*check)(input);
}

int32_t _bvt_run_sequence(bvt_t* n, bvt_callback_table* t, void* input)
{
    BVT_DEBUG_LOG(n);

    bvt_t* c = n->condition;
    while (c) {
        if (_bvt_run(c, t, input) != BVT_SUCCESS)
            return BVT_BACKTRACK;
        c = c->next;
    }

    c = n->child;
    if (!c)
        return BVT_SEQUENCE_ERROR;
    while (c) {
        int32_t ret = _bvt_run(c, t, input);
        if (ret != BVT_SUCCESS)
            return ret;
        c = c->next;
    };
    return BVT_SUCCESS;
}

int32_t _bvt_run_parallel(bvt_t* n, bvt_callback_table* t, void* input)
{
    BVT_DEBUG_LOG(n);

    bvt_t* c = n->condition;
    while (c) {
        if (_bvt_run(c, t, input) != BVT_SUCCESS)
            return BVT_BACKTRACK;
        c = c->next;
    }

    c = n->child;
    if (!c)
        return BVT_PARALLEL_ERROR;
    int32_t ret = BVT_ERROR;
    while (c) {
        int32_t cret = _bvt_run(c, t, input);
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

int32_t _bvt_run_action(bvt_t* n, bvt_callback_table* t, void* input)
{
    BVT_DEBUG_LOG(n);

    if (n->act_args.callback_id < 0 ||
        n->act_args.callback_id >= t->size)
        return BVT_ACTION_ERROR;

    bvt_callback action = t->table[n->act_args.callback_id];
    return (*action)(input);
}

void _bvt_release_node(struct bvt_t* n)
{
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
        free(n);
    }
}

void bvt_release(struct bvt_t* n)
{
    if (n) {
        if (n->cb_table) {
            if (n->cb_table->table)
                free(n->cb_table->table);
            free(n->cb_table);
            n->cb_table = NULL;
        }
        _bvt_release_node(n);
    }
}

static const char* const BVT_CFG_STR_NAME = "name";
static const char* const BVT_CFG_STR_TYPE = "type";
static const char* const BVT_CFG_STR_CHILD = "child";
static const char* const BVT_CFG_STR_COND = "condition";
static const char* const BVT_CFG_STR_ACT = "action";

static const char* const BVT_CFG_TYPE_STR_SEL = "SEL";
static const char* const BVT_CFG_TYPE_STR_SEQ = "SEQ";
static const char* const BVT_CFG_TYPE_STR_PAR = "PAR";
static const char* const BVT_CFG_TYPE_STR_PAR_ALL = "PAR_ALL";
static const char* const BVT_CFG_TYPE_STR_PAR_ONE = "PAR_ONE";
static const char* const BVT_CFG_TYPE_STR_COND = "COND";
static const char* const BVT_CFG_TYPE_STR_ACT = "ACT";

int32_t _bvt_init_node(struct bvt_t* n, cJSON* js)
{
    #define BVT_TYPE_CHECK_ASSIGN(node, t) \
        if (node->type != 0 && node->type != t) return BVT_CONFIG_TYPE_ERROR; \
        node->type = t

    // object
    if (js->type == cJSON_Object) {
        if (js->child)
            return _bvt_init_node(n, js->child);
        return BVT_ERROR;
    }

    // name
    else if (js->type == cJSON_String && 0 == strcmp(js->string, BVT_CFG_STR_NAME)) {
        snprintf(n->name, sizeof(n->name), "%s", js->valuestring);
    }

    // type
    else if (js->type == cJSON_String && 0 == strcmp(js->string, BVT_CFG_STR_TYPE)) {
        if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_SEL)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_SELECTOR);
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_SEQ)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_SEQUENCE);
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_PAR)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_PARALLEL);
            n->par_args.type = BVT_PARALLEL_ALL;
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_PAR_ALL)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_PARALLEL);
            n->par_args.type = BVT_PARALLEL_ALL;
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_PAR_ONE)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_PARALLEL);
            n->par_args.type = BVT_PARALLEL_ONE;
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_COND)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_CONDITION);
        } else if (0 == strcmp(js->valuestring, BVT_CFG_TYPE_STR_ACT)) {
            BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_ACTION);
        }
    }

    // condition
    else if (js->type == cJSON_Number && 0 == strcmp(js->string, BVT_CFG_STR_COND)) {
         BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_CONDITION);
         n->con_args.callback_id = js->valueint;
    }

    // action
    else if (js->type == cJSON_Number && 0 == strcmp(js->string, BVT_CFG_STR_ACT)) {
        BVT_TYPE_CHECK_ASSIGN(n, BVT_NODE_ACTION);
        n->act_args.callback_id = js->valueint;
    }

    // child
    else if ((js->type == cJSON_Array || js->type == cJSON_Object)
        && 0 == strcmp(js->string, BVT_CFG_STR_CHILD)) {
        cJSON* c = js->child;
        if (!c) return BVT_CONFIG_ERROR;
        cJSON* start = c;
        bvt_t* last_child = NULL;
        bvt_t* last_condition = NULL;
        do {
            bvt_t* child = (bvt_t*)malloc(sizeof(bvt_t));
            memset(child, 0, sizeof(bvt_t));

            // recursive loop load
            int32_t ret = _bvt_init_node(child, c);

            // node link
            if (child->type != BVT_NODE_CONDITION) {
                if (!n->child)
                    n->child = child;
                if (last_child)
                    last_child->next = child;
                last_child = child;
            } else {
                if (!n->condition)
                    n->condition = child;
                if (last_condition)
                    last_condition->next = child;
                last_condition = child;
            }

            if (ret != BVT_SUCCESS)
                return ret;
            c = c->next;
        } while (c && c != start);
    }

    // error
    else {
        printf("unrecognized json node[%s]\n", js->string);
        return BVT_CONFIG_NAME_ERROR;
    }

    #undef BVT_TYPE_CHECK_ASSIGN

    // go next
    if (js->next) {
        return _bvt_init_node(n, js->next);
    }
    return BVT_SUCCESS;
}

// json config file
struct bvt_t* bvt_init(const char* cfg)
{
    if (!cfg) return NULL;
    int fd = open(cfg, O_RDONLY);
    if (fd < 0) return NULL;
    off_t size = lseek(fd, 0, SEEK_END);
    char* src = (char*)malloc(size + 1);
    lseek(fd, 0, SEEK_SET);
    read(fd, src, size);
    src[size] = 0;

    // read json config
    cJSON* js = cJSON_Parse(src);
    if (!js) {
        printf("json config %s error\n", cfg);
        return NULL;
    }
    // printf("%s", cJSON_Print(js));

    bvt_t* root = (bvt_t*)malloc(sizeof(bvt_t));
    memset(root, 0, sizeof(bvt_t));
    int32_t ret = _bvt_init_node(root, js->child);
    if (ret != BVT_SUCCESS) {
        _bvt_release_node(root);
        root = NULL;
    }
    cJSON_Delete(js);
    free(src);
    return root;
}

int32_t bvt_register_callback(struct bvt_t* n, bvt_callback cb, int32_t id)
{
    if (!n || !cb || id < 0)
        return BVT_ERROR;

    if (!n->cb_table) {
        n->cb_table = (bvt_callback_table*)malloc(sizeof(bvt_callback_table));
        memset(n->cb_table, 0, sizeof(bvt_callback_table));
        n->cb_table->size = BVT_DEFAULT_TABLE_SIZE > id ? BVT_DEFAULT_TABLE_SIZE : (id + 1);
        n->cb_table->table = (bvt_callback*)malloc(sizeof(bvt_callback) * n->cb_table->size);
        memset(n->cb_table->table, 0, sizeof(bvt_callback) * n->cb_table->size);
    } else if (n->cb_table->size <= id) {
        bvt_callback* c = n->cb_table->table;
        size_t oldsize = n->cb_table->size;
        while (n->cb_table->size <= id) {
            n->cb_table->size *= 2;
        }
        n->cb_table->table = (bvt_callback*)malloc(sizeof(bvt_callback) * n->cb_table->size);
        memset(n->cb_table->table, 0, sizeof(bvt_callback) * n->cb_table->size);
        memcpy(n->cb_table->table, c, sizeof(bvt_callback) * oldsize);
    }

    if (n->cb_table->table[id])
        return BVT_CALLBACK_DUPLICATED;
    n->cb_table->table[id] = cb;
    return BVT_SUCCESS;
}

int32_t _bvt_run(bvt_t* n, bvt_callback_table* t, void* input)
{
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

int32_t bvt_run(struct bvt_t* n, void* input)
{
    if (n) {
        return _bvt_run(n, n->cb_table, input);
    }
    return BVT_ERROR;
}

