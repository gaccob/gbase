#ifndef CON_HASH_H_
#define CON_HASH_H_

//
// for consistant hash
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#if !defined OS_WIN

#include "list.h"
#include "hash.h"

struct conhash_t;

struct conhash_t* conhash_init(hash_func key_hash, hash_func node_hash);
void conhash_release(struct conhash_t* ch);
int32_t conhash_add_node(struct conhash_t* ch, void* node);
void conhash_erase_node(struct conhash_t* ch, void* node);
void* conhash_node(struct conhash_t* ch, void* key);

#endif

#ifdef _cplusplus
}
#endif

#endif // CON_HASH_H_
