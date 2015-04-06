#include <assert.h>
#include "base/conhash.h"

struct key_t {
    char key[32];
};
struct node_t {
    char name[64];
};

static uint32_t
_conhash_key_hash(const void* key) {
    const struct key_t* k = (const struct key_t*)key;
    return hash_jhash(k->key, strlen(k->key));
}

static uint32_t
_conhash_node_hash(const void* node) {
    const struct node_t* n = (const struct node_t*)node;
    return hash_jhash(n->name, strlen(n->name));
}

static int _node_count = 4;
static int _key_count = 10;

int
test_base_conhash(char* param) {
    conhash_t* ch;
    struct node_t node[10];
    struct key_t k;
    ch = conhash_create(_conhash_key_hash, _conhash_node_hash);
    assert(ch);
    for (int i = 0; i < _node_count; ++ i) {
        snprintf(node[i].name, sizeof(node[i].name), "node_%d", i);
        if (conhash_add(ch, &node[i]) < 0) {
            fprintf(stderr, "con-hash add node fail\n");
            conhash_release(ch);
            return -1;
        }
    }
    printf("%d conhash nodes\n", _node_count);

#define _do_hash() \
    for (int i = 0; i < _key_count; ++ i) { \
        snprintf(k.key, sizeof(k.key), "key_%d", i); \
        struct node_t* n = conhash_node(ch, &k); \
        if (!n) { \
            fprintf(stderr, "con-hash get node fail\n"); \
            conhash_release(ch); \
            return -1; \
        } \
        printf("%s -> %s\n", k.key, n->name); \
    }

    _do_hash();

    printf("remove conhash node[0]\n");
    conhash_erase(ch, &node[0]);

    _do_hash();

    printf("add conhash node[0]\n");
    conhash_add(ch, &node[0]);

    _do_hash();

    conhash_release(ch);
    return 0;
}

