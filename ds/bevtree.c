#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
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

static const char* const BVT_GLIFFY_TYPE_STAGE = "stage";
static const char* const BVT_GLIFFY_TYPE_OBJECTS = "objects";
static const char* const BVT_GLIFFY_TYPE_ID = "id";
static const char* const BVT_GLIFFY_TYPE_GRAPHIC = "graphic";
static const char* const BVT_GLIFFY_TYPE_TYPE = "type";
static const char* const BVT_GLIFFY_TYPE_CHILDREN = "children";
static const char* const BVT_GLIFFY_TYPE_HTML = "html";
static const char* const BVT_GLIFFY_TYPE_TEXT = "Text";
static const char* const BVT_GLIFFY_TYPE_CONSTRAINTS = "constraints";
static const char* const BVT_GLIFFY_TYPE_CONSTRAINTS_START = "startConstraint";
static const char* const BVT_GLIFFY_TYPE_CONSTRAINTS_END = "endConstraint";
static const char* const BVT_GLIFFY_TYPE_CONSTRAINTS_START_POS = "StartPositionConstraint";
static const char* const BVT_GLIFFY_TYPE_CONSTRAINTS_END_POS = "EndPositionConstraint";
static const char* const BVT_GLIFFY_TYPE_NODEID = "nodeId";

static const char* const BVT_GLIFFY_VALUE_SHAPE = "Shape";
static const char* const BVT_GLIFFY_VALUE_LINE = "Line";
static const char* const BVT_GLIFFY_VALUE_TEXT = "Text";

#define BVT_MAX_GRAPH_DESC_LEN 32
enum {
    BVT_GRAPH_NODE_SHAPE,
    BVT_GRAPH_NODE_LINE,
};
typedef struct bvt_graph_node_t
{
    int32_t t;
    int32_t id;
    union {
        struct {
            char desc[BVT_MAX_GRAPH_DESC_LEN];
        } shape;
        struct {
            int32_t from;
            int32_t to;
        } line;
    };
    struct bvt_graph_node_t* next;
} bvt_graph_node_t;

typedef struct bvt_graph_t {
    struct bvt_graph_node_t* head;
} bvt_graph_t;

// trim html tags
void _bvt_load_gliffy_html(const char* html, char* dst, size_t dstlen)
{
    assert(html);
    int32_t flag = 0;
    int32_t trans = 0;
    size_t dlen = 0;
    while (html) {
        if (dlen >= dstlen - 1) break;
        if (trans == 1) { trans = 0; ++ html; continue; }
        if (*html == '<') { ++ flag; ++ html; continue; }
        if (*html == '\\') { trans = 1; ++ html; continue; }
        if (*html == '>' && flag > 0) { -- flag; ++ html; continue; }
        if (flag == 0) { dst[dlen++] = *html; }
        ++html;
    }
    dst[dlen] = 0;
}

#undef BVT_JSON_GO_DOWN
#define BVT_JSON_GO_DOWN(js, jt, name) \
    do { \
        (js) = (js)->child; \
        while (js) { \
            if ((js)->type == (jt) && 0 == strcmp((js)->string, (name))) \
                break; \
            (js) = (js)->next; \
        } \
    } while (0)

bvt_graph_node_t* _bvt_load_gliffy_node(cJSON* js)
{
    cJSON* js_id = js;
    BVT_JSON_GO_DOWN(js_id, cJSON_Number, BVT_GLIFFY_TYPE_ID);
    if (!js_id) return NULL;

    cJSON* js_graphic = js;
    BVT_JSON_GO_DOWN(js_graphic, cJSON_Object, BVT_GLIFFY_TYPE_GRAPHIC);
    if (!js_graphic) return NULL;

    cJSON* js_type = js_graphic;
    BVT_JSON_GO_DOWN(js_type, cJSON_String, BVT_GLIFFY_TYPE_TYPE);
    if (!js_type) return NULL;

    bvt_graph_node_t* node = (bvt_graph_node_t*)malloc(sizeof(bvt_graph_node_t));
    node->id = js_id->valueint;
    node->next = NULL;

    // load shape desc
    if (0 == strcmp(js_type->valuestring, BVT_GLIFFY_VALUE_SHAPE)) {
        node->t = BVT_GRAPH_NODE_SHAPE;

        // children list
        cJSON* js_children = js;
        BVT_JSON_GO_DOWN(js_children, cJSON_Array, BVT_GLIFFY_TYPE_CHILDREN);
        if (!js_children) goto GLIFFY_FAIL;

        // child object
        cJSON* c = js_children->child;
        if (!c) goto GLIFFY_FAIL;

        while (c) {
            // child graphic
            cJSON* js_child_graphic = c;
            BVT_JSON_GO_DOWN(js_child_graphic, cJSON_Object, BVT_GLIFFY_TYPE_GRAPHIC);
            if (!js_child_graphic) goto GLIFFY_FAIL;

            // child graphic type
            cJSON* js_child_type = js_child_graphic;
            BVT_JSON_GO_DOWN(js_child_type, cJSON_String, BVT_GLIFFY_TYPE_TYPE);
            if (!js_child_type) goto GLIFFY_FAIL;
            if (strcmp(js_child_type->valuestring, BVT_GLIFFY_VALUE_TEXT)) {
                c = c->next;
                continue;
            }

            // child graphic text
            cJSON* js_child_text = js_child_graphic;
            BVT_JSON_GO_DOWN(js_child_text, cJSON_Object, BVT_GLIFFY_TYPE_TEXT);
            if (!js_child_text) goto GLIFFY_FAIL;

            // child graphic text html
            cJSON* js_child_text_html = js_child_text;
            BVT_JSON_GO_DOWN(js_child_text_html, cJSON_String, BVT_GLIFFY_TYPE_HTML);
            if (!js_child_text_html) goto GLIFFY_FAIL;

            // set shape value
            _bvt_load_gliffy_html(js_child_text_html->valuestring, node->shape.desc,
                sizeof(node->shape.desc));
            break;
        }
        if (!c) goto GLIFFY_FAIL;
    }

    // load line end points
    else if (0 == strcmp(js_type->valuestring, BVT_GLIFFY_VALUE_LINE)) {

        node->t = BVT_GRAPH_NODE_LINE;

        // child constraints
        cJSON* js_cst = js;
        BVT_JSON_GO_DOWN(js_cst, cJSON_Object, BVT_GLIFFY_TYPE_CONSTRAINTS);
        if (!js_cst) goto GLIFFY_FAIL;

        // child constraints start
        cJSON* js_cst_start = js_cst;
        BVT_JSON_GO_DOWN(js_cst_start, cJSON_Object, BVT_GLIFFY_TYPE_CONSTRAINTS_START);
        if (!js_cst_start) goto GLIFFY_FAIL;
        cJSON* js_cst_start_pos = js_cst_start;
        BVT_JSON_GO_DOWN(js_cst_start_pos, cJSON_Object, BVT_GLIFFY_TYPE_CONSTRAINTS_START_POS);
        if (!js_cst_start_pos) goto GLIFFY_FAIL;
        cJSON* js_cst_start_id = js_cst_start_pos;
        BVT_JSON_GO_DOWN(js_cst_start_id, cJSON_Number, BVT_GLIFFY_TYPE_NODEID);
        if (!js_cst_start_id) goto GLIFFY_FAIL;
        node->line.from = js_cst_start_id->valueint;

        // child constaints end
        cJSON* js_cst_end = js_cst;
        BVT_JSON_GO_DOWN(js_cst_end, cJSON_Object, BVT_GLIFFY_TYPE_CONSTRAINTS_END);
        if (!js_cst_end) goto GLIFFY_FAIL;
        cJSON* js_cst_end_pos = js_cst_end;
        BVT_JSON_GO_DOWN(js_cst_end_pos, cJSON_Object, BVT_GLIFFY_TYPE_CONSTRAINTS_END_POS);
        if (!js_cst_end_pos) goto GLIFFY_FAIL;
        cJSON* js_cst_end_id = js_cst_end_pos;
        BVT_JSON_GO_DOWN(js_cst_end_id, cJSON_Number, BVT_GLIFFY_TYPE_NODEID);
        if (!js_cst_end_id) goto GLIFFY_FAIL;
        node->line.to = js_cst_end_id->valueint;
    }

    // unrecognized gliffy type
    else {
        goto GLIFFY_FAIL;
    }

    return node;

GLIFFY_FAIL:
    free(node);
    return NULL;
}

int32_t _bvt_load_gliffy_parse_name(bvt_t* node, char* name) {
    if (!node || !name) return BVT_ERROR;
    const char* split = BVT_GLIFFY_SPLIT;

    // type
    char* p = strtok(name, split);
    if (!p) return BVT_ERROR;
    if (0 == strcmp(p, "SEQ")) {
        node->type = BVT_NODE_SEQUENCE;
    } else if (0 == strcmp(p, "SEL")) {
        node->type = BVT_NODE_SELECTOR;
    } else if (0 == strcmp(p, "PAR")) {
        node->type = BVT_NODE_PARALLEL;
    } else if (0 == strcmp(p, "ACT")) {
        node->type = BVT_NODE_ACTION;
    } else if (0 == strcmp(p, "COND")) {
        node->type = BVT_NODE_CONDITION;
    } else if (0 == strcmp(p, "PAR_ALL")) {
        node->type = BVT_NODE_PARALLEL;
        node->par_args.type = BVT_PARALLEL_ALL;
    } else if (0 == strcmp(p, "PAR_ONE")) {
        node->type = BVT_NODE_PARALLEL;
        node->par_args.type = BVT_PARALLEL_ONE;
    } else {
        return BVT_ERROR;
    }

    // desc
    p = strtok(NULL, split);
    snprintf(node->name, sizeof(node->name), "%s", p);

    // callback id
    p = strtok(NULL, split);
    if (p) {
        if (node->type == BVT_NODE_ACTION)
            node->act_args.callback_id = atoi(p);
        else if (node->type == BVT_NODE_CONDITION)
            node->con_args.callback_id = atoi(p);
        else
            return BVT_ERROR;
    }

    return BVT_SUCCESS;
}

bvt_t* _bvt_load_gliffy_graph_node(int32_t id, bvt_graph_node_t* list)
{
    if (!list) return NULL;

    // find and split a node
    bvt_graph_node_t* node = list;
    bvt_graph_node_t* prev = 0;
    while (node) {
        if (node->t == BVT_GRAPH_NODE_SHAPE && node->id == id) {
            if (prev) prev->next = node->next;
            else list = list->next;
            break;
        }
        prev = node;
        node = node->next;
    }
    if (!node) return NULL;

    // set node data
    bvt_t* b = (bvt_t*)malloc(sizeof(bvt_t));
    memset(b, 0, sizeof(bvt_t));
    int32_t ret = _bvt_load_gliffy_parse_name(b, node->shape.desc);
    if (ret != BVT_SUCCESS) {
        printf("%s error\n", node->shape.desc);
        assert(0);
    }
    // release graph nodes
    free(node);

    // find its children
    while (1) {
        bvt_graph_node_t* line = list;
        prev = 0;
        while (line) {
            if (line->t == BVT_GRAPH_NODE_LINE && line->line.from == id) {
                if (prev) prev->next = line->next;
                else list = list->next;
                break;
            }
            prev = line;
            line = line->next;
        }
        if (!line) break;

        bvt_t* child = _bvt_load_gliffy_graph_node(line->line.to, list);
        assert(child);
        if (child->type == BVT_NODE_CONDITION) {
            if (b->condition) {
                child->next = b->condition;
            }
            b->condition = child;
        } else {
            if (b->child) {
                child->next = b->child;
            }
            b->child = child;
        }

        // release graph nodes
        free(line);
    }
    return b;
}

void _bvt_debug(bvt_t* b, int32_t indent)
{
    int32_t i = indent;
    while (i --) { printf("——"); }
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
    bvt_t* c = b->condition;
    while (c) {
        _bvt_debug(c, indent + 1);
        c = c->next;
    }
    c = b->child;
    while (c) {
        _bvt_debug(c, indent + 1);
        c = c->next;
    }
}

void bvt_debug(bvt_t* b) {
    if (b) {
        printf("\n================\n");
        _bvt_debug(b, 0);
        printf("\n\n");
    }
}

void _bvt_debug_graph(bvt_graph_t* g)
{
    // printf debug
    bvt_graph_node_t* n = g->head;
    while (n) {
        if (n->t == BVT_GRAPH_NODE_SHAPE) {
            printf("shape[%d]: %s\n", n->id, n->shape.desc);
        } else if (n->t == BVT_GRAPH_NODE_LINE) {
            printf("line[%d->%d]\n", n->line.from, n->line.to);
        }
        n = n->next;
    }
}

// init bvt by graph
bvt_t* _bvt_load_gliffy_graph(bvt_graph_t* g)
{
    // get root id
    bvt_graph_node_t* n = g->head;
    while (n) {
        if (n->t == BVT_GRAPH_NODE_LINE) {
            n = n->next;
            continue;
        }
        bvt_graph_node_t* b = g->head;
        while (b) {
            if (b->t == BVT_GRAPH_NODE_LINE && b->line.to == n->id) {
                break;
            }
            b = b->next;
        }
        if (!b) break;
    }
    assert(n);
    printf("root id:%d\n", n->id);

    return _bvt_load_gliffy_graph_node(n->id, g->head);
}

// gliffy is also json format, but with lots of view info
// we need to get what we need
bvt_t* _bvt_load_gliffy(cJSON* js)
{
    if (js->type != cJSON_Object)
        return NULL;

    // stage node
    BVT_JSON_GO_DOWN(js, cJSON_Object, BVT_GLIFFY_TYPE_STAGE);
    if (!js) return NULL;

    // objects node
    BVT_JSON_GO_DOWN(js, cJSON_Array, BVT_GLIFFY_TYPE_OBJECTS);
    if (!js) return NULL;

    // load temporary graph
    bvt_graph_t g;
    memset(&g, 0, sizeof(g));
    bvt_graph_node_t* head = g.head;
    cJSON* c = js->child;
    while (c) {
        if (c->type == cJSON_Object) {
            bvt_graph_node_t* node = _bvt_load_gliffy_node(c);
            if (node) {
                if (head) {
                    head->next = node;
                    head = node;
                } else {
                    g.head = node;
                    head = node;
                }
            }
        }
        c = c->next;
    }

    // init bvt by graph
    return _bvt_load_gliffy_graph(&g);
}

// init by gliffy file (also json format, but with lots of vision info)
struct bvt_t* bvt_load_gliffy(const char* cfg)
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
        printf("gliffy config %s error\n", cfg);
        return NULL;
    }

    bvt_t* root = _bvt_load_gliffy(js);
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

