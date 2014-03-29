#ifndef HASH_H_
#define HASH_H_

//
// it's a hash table (not key-value style, just a table)
// with a jhash method
//

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

typedef uint32_t (*hash_func)(const void*);
typedef int32_t (*hash_cmp_func)(const void*, const void*);
typedef void (*hash_loop_func)(void* data, void* args);
struct hash_t;

struct hash_t* hash_create(hash_func hash, hash_cmp_func cmp, int32_t hint_size);
int32_t hash_release(struct hash_t* htable);

int32_t hash_clean(struct hash_t* htable);
int32_t hash_insert(struct hash_t* htable, void* data);
int32_t hash_remove(struct hash_t* htable, void* data);
int32_t hash_count(struct hash_t* htable);
void hash_loop(struct hash_t* htable, hash_loop_func func, void* args);
void* hash_find(struct hash_t* htable, void* data);

/* jhash.h: Jenkins hash support.
 *
 * Copyright (C) 1996 Bob Jenkins (bob_jenkins@burtleburtle.net)
 *
 * http://burtleburtle.net/bob/hash/
 *
 * These are the credits from Bob's sources:
 *
 * lookup2.c, by Bob Jenkins, December 1996, Public Domain.
 * hash(), hash2(), hash3, and mix() are externally useful functions.
 * Routines to test the hash are included if SELF_TEST is defined.
 * You can use this free for any purpose.  It has no warranty.
 *
 * Copyright (C) 2003 David S. Miller (davem@redhat.com)
 *
 * I've modified Bob's hash to be useful in the Linux kernel, and
 * any bugs present are surely my fault.  -DaveM
 */
uint32_t hash_jhash(const void* key, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif // HASH_H_



