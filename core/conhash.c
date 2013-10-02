#include "conhash.h"

#if !defined OS_WIN

typedef struct conhash_t
{
    struct list_head node_list;
    hash_func key_hash;
    hash_func node_hash;
} conhash_t;

#define CONHASH_NODE_NAME_SIZE 128

struct conhash_node_t
{
    struct list_head link;
    void* data;
    uint32_t hash_value;
};

conhash_t* conhash_init(hash_func key_hash, hash_func node_hash)
{
    conhash_t* ch = NULL;
    if (!key_hash || !node_hash) return NULL;
    ch = (conhash_t*)MALLOC(sizeof(conhash_t));
    if (!ch) return NULL;
    INIT_LIST_HEAD(&ch->node_list);
    ch->key_hash = key_hash;
    ch->node_hash = node_hash;
    return ch;
}

void conhash_release(conhash_t* ch)
{
    struct conhash_node_t* p, *n;
    if (ch) {
        list_for_each_entry_safe(p, n, &ch->node_list, link) {
            FREE(p);
        }
        FREE(ch);
    }
}

int32_t conhash_add_node(conhash_t* ch, void* node)
{
    struct conhash_node_t* new_node, *n;
    if (!ch || !node) return -1;
    new_node = (struct conhash_node_t*)MALLOC(sizeof(struct conhash_node_t));
    new_node->data = node;
    new_node->hash_value = ch->node_hash(node);
    INIT_LIST_HEAD(&new_node->link);
    list_for_each_entry(n, &ch->node_list, link) {
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

void conhash_erase_node(conhash_t* ch, void* node)
{
    uint32_t val;
    struct conhash_node_t* n;
    if (!ch || !node) return;

    val = ch->node_hash(node);
    list_for_each_entry(n, &ch->node_list, link) {
        if (n->hash_value == val) {
            list_del(&n->link);
            FREE(n);
            return;
        }
    }
}

void* conhash_node(conhash_t* ch, void* key)
{
    uint32_t val;
    struct conhash_node_t* n;
    struct list_head* l;
    if (!ch || !key) return NULL;
    if (list_empty(&ch->node_list)) return NULL;

    val = ch->key_hash(key);
    list_for_each_entry(n, &ch->node_list, link) {
        if (n->hash_value >= val) return n->data;
    }
    l = ch->node_list.next;
    n = list_entry(l, struct conhash_node_t, link);
    return n->data;
}

#endif
