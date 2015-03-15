#ifndef KV_H_
#define KV_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "hash.h"

typedef struct hash_t kv_t;

typedef uint32_t (*kv_hash_func)(const void* k);
typedef int (*kv_cmp_func)(const void* k_l, const void* k_r);
typedef void (*kv_loop_func)(void* k, void* v, void* arg);

kv_t* kv_create(kv_hash_func, kv_cmp_func, int hint_size);
int kv_release(kv_t*);

int kv_insert(kv_t*, void* key, void* value);
void* kv_find(kv_t*, void* key);
void* kv_erase(kv_t*, void* key);
int kv_count(kv_t*);
int kv_clean(kv_t*);
void kv_loop(kv_t*, kv_loop_func, void* arg);

#ifdef __cplusplus
}
#endif

#endif
