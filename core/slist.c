#include "slist.h"

typedef struct slist_node_t
{
    struct slist_node_t* next;
    void* data;
} slist_node_t;

typedef struct slist_t
{
    int count;
    struct slist_node_t* node;
} slist_t;

struct slist_t* slist_init()
{
    struct slist_t* sl = (struct slist_t*)MALLOC(sizeof(struct slist_t));
    if (!sl) return NULL;
    sl->node = NULL;
    sl->count = 0;
    return sl;
}

void slist_release(struct slist_t* sl)
{
    if (sl) {
        slist_clean(sl);
        FREE(sl);
    }
}

// more effective than push_back as it's single list
int slist_push_front(struct slist_t* sl, void* data)
{
    struct slist_node_t* new_node;
    if (!sl || !data) return -1;

    new_node = (struct slist_node_t*)MALLOC(sizeof(*new_node));
    if (!new_node) return -1;
    new_node->data = data;
    new_node->next = sl->node;
    sl->count ++;
    sl->node = new_node;
    return 0;
}

int slist_push_back(struct slist_t* sl, void* data)
{
    struct slist_node_t* new_node;
    struct slist_node_t* node;
    if (!sl || !data) return -1;

    new_node = (struct slist_node_t*)MALLOC(sizeof(*new_node));
    if (!new_node) return -1;
    new_node->data = data;
    new_node->next = 0;
    sl->count ++;

    if (!sl->node) {
        sl->node = new_node;
    } else {
        node = sl->node;
        while (node->next) {
            node = node->next;
        }
        node->next = new_node;
    }
    return 0;
}

// more effective than pop_back, as it's single list
void* slist_pop_front(struct slist_t* sl)
{
    struct slist_node_t* n;
    void* data;
    if (!sl || !sl->node) {
        return NULL;
    }
    n = sl->node;
    data = n->data; 
    sl->node = n->next;
    FREE(n);
    sl->count --;
    return data;
}

void* slist_pop_back(struct slist_t* sl)
{
    struct slist_node_t* prev;
    struct slist_node_t* n;
    void* data;
    if (!sl || !sl->node) {
        return NULL;
    }
    n = sl->node;
    prev = NULL;
    while (n->next) {
        prev = n;
        n = n->next;
    }
    if (!prev) {
        sl->node = NULL;
    } else {
        prev->next = NULL;
    }
    data = n->data;
    FREE(n);
    sl->count --;
    return data;
}

int slist_remove(struct slist_t* sl, void* data)
{
    struct slist_node_t *node, *tmp;
    if (!sl || !data) return -1;

    node = sl->node;
    tmp = 0;
    while (node && node->data != data) {
        tmp = node;
        node = node->next;
    }
    if (node) {
        if (0 == tmp) {
            sl->node = node->next;
        } else {
            tmp->next = node->next;
        }
        FREE(node);
        sl->count --;
    }
    return 0;
}

int slist_find(struct slist_t* sl, void* data)
{
    struct slist_node_t* node;
    if (!sl || !data) return -1;
    node = sl->node;
    while (node) {
        if (node->data == data) {
            return 0;
        }
        node = node->next;
    }
    return -1;
}

int slist_clean(struct slist_t* sl)
{
    struct slist_node_t *node, *tmp;
    if (!sl) return -1;
    node = sl->node;
    while (node) {
        tmp = node->next;
        FREE(node);
        node = tmp;
    }
    sl->node = 0;
    return 0;
}

int slist_count(struct slist_t* sl)
{
    return sl ? sl->count : -1;
}


