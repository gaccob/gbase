#include "slist.h"

typedef struct node_t {
    struct node_t* next;
    void* data;
} node_t;

struct slist_t {
    int size;
    node_t* node;
};

slist_t*
slist_create() {
    slist_t* sl = (slist_t*)MALLOC(sizeof(slist_t));
    if (!sl) return NULL;
    sl->node = NULL;
    sl->size = 0;
    return sl;
}

void
slist_release(slist_t* sl) {
    if (sl) {
        slist_clean(sl);
        FREE(sl);
    }
}

// more effective than push_back as it's single list
int
slist_push_front(slist_t* sl, void* data) {
    if (!sl || !data)
        return -1;
    node_t* new_node = (node_t*)MALLOC(sizeof(*new_node));
    if (!new_node) return -1;
    new_node->data = data;
    new_node->next = sl->node;
    sl->size ++;
    sl->node = new_node;
    return 0;
}

int
slist_push_back(slist_t* sl, void* data) {
    if (!sl || !data)
        return -1;
    node_t* new_node = (node_t*)MALLOC(sizeof(*new_node));
    if (!new_node) return -1;
    new_node->data = data;
    new_node->next = 0;
    sl->size ++;

    if (!sl->node) {
        sl->node = new_node;
    } else {
        node_t* node = sl->node;
        while (node->next) {
            node = node->next;
        }
        node->next = new_node;
    }
    return 0;
}

// more effective than pop_back, as it's single list
void*
slist_pop_front(slist_t* sl) {
    if (!sl || !sl->node) {
        return NULL;
    }
    node_t* n = sl->node;
    void* data = n->data;
    sl->node = n->next;
    FREE(n);
    sl->size --;
    return data;
}

void*
slist_pop_back(slist_t* sl) {
    if (!sl || !sl->node) {
        return NULL;
    }
    node_t* n = sl->node;
    node_t* prev = NULL;
    while (n->next) {
        prev = n;
        n = n->next;
    }
    if (!prev) {
        sl->node = NULL;
    } else {
        prev->next = NULL;
    }
    void* data = n->data;
    FREE(n);
    sl->size --;
    return data;
}

int
slist_remove(slist_t* sl, void* data) {
    if (!sl || !data)
        return -1;
    node_t* node = sl->node;
    node_t* tmp = 0;
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
        sl->size --;
    }
    return 0;
}

int
slist_find(slist_t* sl, void* data) {
    if (!sl || !data)
        return -1;
    node_t* node = sl->node;
    while (node) {
        if (node->data == data) {
            return 0;
        }
        node = node->next;
    }
    return -1;
}

int
slist_clean(slist_t* sl) {
    if (!sl)
        return -1;
    node_t* node = sl->node;
    while (node) {
        node_t* tmp = node->next;
        FREE(node);
        node = tmp;
    }
    sl->node = 0;
    return 0;
}

int
slist_size(slist_t* sl) {
    return sl ? sl->size : -1;
}

