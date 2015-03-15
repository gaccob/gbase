#include "hash.h"
#include "kv.h"

typedef struct kv_node_t {
    void* key;
    void* value;
} kv_node_t;

kv_t*
kv_create(kv_hash_func hash, kv_cmp_func cmp, int hint_size) {
    return hash_create(hash, cmp, hint_size);
}

int
kv_release(kv_t* kv) {
    return hash_release(kv);
}

int
kv_insert(kv_t* kv, void* key, void* value) {
    if (!kv || !key) {
        return -1;
    }
    kv_node_t* node = (kv_node_t*) MALLOC(sizeof(kv_node_t));
    node->key = key;
    node->value = value;
    return hash_insert(kv, node);
}

void*
kv_find(kv_t* kv, void* key) {
    if (!key || !key) {
        return NULL;
    }
    kv_node_t* node = (kv_node_t*)hash_find(kv, key);
    return node ? node->value : NULL;
}

void*
kv_erase(kv_t* kv, void* key) {
    if (!kv || !key) {
        return NULL;
    }
    kv_node_t* node = (kv_node_t*)hash_remove(kv, key);
    if (node) {
        void* value = node->value;
        FREE(node);
        return value;
    }
    return NULL;
}

int
kv_count(kv_t* kv) {
    return hash_count(kv);
}

int
kv_clean(kv_t* kv) {
    return hash_clean(kv);
}

typedef struct kv_callback_t {
    kv_loop_func loop;
    void* arg;
} kv_callback_t;

static void
_kv_loop_func(void* data, void* arg) {
    kv_callback_t* cb = (kv_callback_t*)arg;
    kv_node_t* node = (kv_node_t*)data;
    cb->loop(node->key, node->value, cb->arg);
}

void
kv_loop(kv_t* kv, kv_loop_func loop, void* arg) {
    if (kv && loop) {
        kv_callback_t cb;
        cb.loop = loop;
        cb.arg = arg;
        hash_loop(kv, _kv_loop_func, &cb);
    }
}

