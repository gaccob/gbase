#include "base/list.h"
#include "conhash.h"

struct conhash_t {
    list_head_t node_list;
    hash_func key_hash;
    hash_func node_hash;
};

#define CONHASH_NODE_NAME_SIZE 128

typedef struct conhash_node_t {
    list_head_t link;
    void* data;
    uint32_t hash_value;
} conhash_node_t;

conhash_t*
conhash_create(hash_func key_hash, hash_func node_hash) {
    if (!key_hash || !node_hash)
        return NULL;
    conhash_t* ch = (conhash_t*)MALLOC(sizeof(conhash_t));
    if (!ch)
        return NULL;
    INIT_LIST_HEAD(&ch->node_list);
    ch->key_hash = key_hash;
    ch->node_hash = node_hash;
    return ch;
}

void
conhash_release(conhash_t* ch) {
    if (ch) {
        conhash_node_t* p, *n;
        list_for_each_entry_safe(p, conhash_node_t, n, &ch->node_list, link) {
			FREE(p);
        }
        FREE(ch);
    }
}

int
conhash_add(conhash_t* ch, void* node) {
    if (!ch || !node)
        return -1;
    conhash_node_t* new_node = (conhash_node_t*)MALLOC(sizeof(conhash_node_t));
    new_node->data = node;
    new_node->hash_value = ch->node_hash(node);
    INIT_LIST_HEAD(&new_node->link);
    conhash_node_t* n;
    list_for_each_entry(n, conhash_node_t, &ch->node_list, link) {
        if (n->hash_value == new_node->hash_value) {
            FREE(new_node);
            return -1;
        } else if (n->hash_value > new_node->hash_value) {
            list_add(&new_node->link, n->link.prev);
            return 0;
        }
    }
    list_add(&new_node->link, ch->node_list.prev);
    return 0;
}

void
conhash_erase(conhash_t* ch, void* node) {
    if (!ch || !node)
        return;
    uint32_t val = ch->node_hash(node);
    conhash_node_t* n;
    list_for_each_entry(n, conhash_node_t, &ch->node_list, link) {
        if (n->hash_value == val) {
            list_del(&n->link);
            FREE(n);
            return;
        }
    }
}

void*
conhash_node(conhash_t* ch, void* key) {
    if (!ch || !key)
        return NULL;
    if (list_empty(&ch->node_list))
        return NULL;
    uint32_t val = ch->key_hash(key);
    conhash_node_t* n;
    list_for_each_entry(n, conhash_node_t, &ch->node_list, link) {
        if (n->hash_value >= val) return n->data;
    }
    list_head_t* l = ch->node_list.next;
    n = list_entry(l, conhash_node_t, link);
    return n->data;
}

