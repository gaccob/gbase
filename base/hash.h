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
typedef int (*hash_cmp_func)(const void*, const void*);
typedef void (*hash_loop_func)(void* data, void* args);
typedef struct hash_t hash_t;

hash_t* hash_create(hash_func hash, hash_cmp_func cmp, int hint_size);
int hash_release(hash_t*);

int hash_clean(hash_t*);
int hash_insert(hash_t*, void* data);
int hash_remove(hash_t*, void* data);
int hash_count(hash_t*);
void hash_loop(hash_t*, hash_loop_func func, void* args);
void* hash_find(hash_t*, void* data);

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



