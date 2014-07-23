#ifndef CON_HASH_H_
#define CON_HASH_H_

//
// for consistant hash
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"
#include "base/hash.h"

typedef struct conhash_t conhash_t;

conhash_t* conhash_create(hash_func key_hash, hash_func node_hash);
void conhash_release(conhash_t*);
int conhash_add(conhash_t*, void* node);
void conhash_erase(conhash_t*, void* node);
void* conhash_node(conhash_t*, void* key);

#ifdef __cplusplus
}
#endif

#endif // CON_HASH_H_
