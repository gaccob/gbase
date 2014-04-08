#include <assert.h>
#include "base/conhash.h"

struct key_t {
    char key[32];
};
struct node_t {
    char name[64];
};

uint32_t
conhash_key_hash(const void* key) {
    const struct key_t* k = (const struct key_t*)key;
    return hash_jhash(k->key, strlen(k->key));
}

uint32_t
conhash_node_hash(const void* node) {
    const struct node_t* n = (const struct node_t*)node;
    return hash_jhash(n->name, strlen(n->name));
}

int32_t
test_conhash() {
    struct conhash_t* ch = conhash_create(conhash_key_hash, conhash_node_hash);
    assert(ch);
    struct node_t node[10];
    struct node_t* n;
    int32_t i, ret;
    for (i=0; i<4; i++) {
        snprintf(node[i].name, sizeof(node[i].name), "node_%d", i);
        ret = conhash_add_node(ch, &node[i]);
        assert(0 == ret);
    }
    for (i=0; i<10; i++) {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_erase_node(ch, &node[0]);
    for (i=0; i<10; i++) {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_add_node(ch, &node[0]);
    for (i=0; i<10; i++) {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    conhash_release(ch);
    return 0;
}

