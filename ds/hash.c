#include <assert.h>
#include <string.h>
#include "ds/hash.h"

typedef struct hash_node_t
{
    void* m_data;
    struct hash_node_t* m_next;
}hash_node_t;

typedef struct hash_t
{
    int32_t m_size;
    int32_t m_count;
    hash_func m_hash_func;
    cmp_func m_cmp_func;
    struct hash_node_t** m_table;
}hash_t;

struct hash_t* hash_init(hash_func hash, cmp_func cmp, int32_t hint_size)
{
    struct hash_t* htable;
    if (!hash || !cmp || hint_size <= 0) {
        return NULL;
    }

    htable = (struct hash_t*)MALLOC(sizeof(struct hash_t));
    if (!htable) {
        goto HASH_FAIL;
    }
    memset(htable, 0, sizeof(*htable));
    htable->m_size = hint_size;
    htable->m_cmp_func = cmp;
    htable->m_hash_func = hash;

    htable->m_table = (hash_node_t**)MALLOC(sizeof(hash_node_t*) * htable->m_size);
    if (!htable->m_table) {
        goto HASH_FAIL1;
    }
    memset(htable->m_table, 0, sizeof(hash_node_t*) * htable->m_size);
    return htable;

HASH_FAIL1:
    FREE(htable);
HASH_FAIL:
    return NULL;
}

int32_t hash_release(struct hash_t* htable)
{
    if (!htable) return -1;
    hash_clean(htable);
    FREE(htable->m_table);
    FREE(htable);
    return 0;
}

int32_t hash_clean(struct hash_t* htable)
{
    int32_t i;
    hash_node_t* bak;
    hash_node_t* node;

    if (!htable) return -1;
    for (i = 0; i < htable->m_size; ++ i) {
        // free list node
        node = htable->m_table[i];
        bak = 0;
        while (node) {
            bak = node;
            node->m_data = 0;
            node = node->m_next;
            FREE(bak);
            bak = 0;
        }
        htable->m_table[i] = 0;
    }
    return 0;
}

void hash_loop(struct hash_t* htable, loop_func f, void* args)
{
    int32_t i;
    hash_node_t* node;
    if (!htable || !f) return;

    for (i = 0; i < htable->m_size; ++ i) {
        node = htable->m_table[i];
        while (node) {
            if (node->m_data) {
                (*f)(node->m_data, args);
            }
            node = node->m_next;
        }
    }
}

int32_t hash_insert(struct hash_t* htable, void* data)
{
    uint32_t hash_key, index;
    hash_node_t* node;
    hash_node_t* prev;

    if (!htable || !data) return -1;
    hash_key = htable->m_hash_func(data);
    index = hash_key % htable->m_size;
    node = htable->m_table[index];
    prev = 0;
    while (node) {
        // exist items
        if (0 == htable->m_cmp_func(node->m_data, data)) {
            return -1;
        }
        prev = node;
        node = node->m_next;
    }

    node = (hash_node_t*)MALLOC(sizeof(struct hash_node_t));
    if (!node) return -1;
    node->m_data = data;
    node->m_next = 0;
    if (prev) {
        prev->m_next = node;
    } else {
        htable->m_table[index] = node;
    }
    htable->m_count ++;
    return 0;
}

int32_t hash_remove(struct hash_t* htable, void* data)
{
    uint32_t hash_key, index;
    hash_node_t* node;
    hash_node_t* prev;

    if (!htable || !data) return -1;
    hash_key = htable->m_hash_func(data);
    index = hash_key % htable->m_size;
    node = htable->m_table[index];
    prev = 0;

    while (node) {
        if (0 == htable->m_cmp_func(node->m_data, data)) {
            if (prev) {
                prev->m_next = node->m_next;
            } else {
                htable->m_table[index] = 0;
            }
            FREE(node);
            node = 0;
            htable->m_count --;
            return 0;
        }
        prev = node;
        node = node->m_next;
    }
    return -1;
}

int32_t hash_count(struct hash_t* htable)
{
    if (!htable) return -1;
    return htable->m_count;
}


void* hash_find(struct hash_t* htable, void* data)
{
    uint32_t hash_key;
    struct hash_node_t* node;

    if (!htable || !data) return NULL;
    hash_key = htable->m_hash_func(data);
    node = htable->m_table[hash_key % htable->m_size];
    while (node) {
        if (0 == htable->m_cmp_func(node->m_data, data)) {
            return node->m_data;
        }
        node = node->m_next;
    }
    return NULL;
}

// NOTE: Arguments are modified.
#define __jhash_mix(a, b, c) \
    { \
        a -= b; a -= c; a ^= (c>>13); \
        b -= c; b -= a; b ^= (a<<8); \
        c -= a; c -= b; c ^= (b>>13); \
        a -= b; a -= c; a ^= (c>>12);  \
        b -= c; b -= a; b ^= (a<<16); \
        c -= a; c -= b; c ^= (b>>5); \
        a -= b; a -= c; a ^= (c>>3);  \
        b -= c; b -= a; b ^= (a<<10); \
        c -= a; c -= b; c ^= (b>>15); \
    }

// The golden ration: an arbitrary value
#define JHASH_GOLDEN_RATIO  0x9e3779b9

// The most generic version, hashes an arbitrary sequence
// of bytes.  No alignment or length assumptions are made about
// the input key.
uint32_t hash_jhash(const void* key, uint32_t length)
{
    uint32_t a, b, c, len;
    const unsigned char* k = (const unsigned char*)key;

    len = length;
    a = b = JHASH_GOLDEN_RATIO;
    c = 99989;

    while (len >= 12) {
        a += (k[0] + ((uint32_t) k[1] << 8) + ((uint32_t) k[2] << 16) +
              ((uint32_t) k[3] << 24));
        b += (k[4] + ((uint32_t) k[5] << 8) + ((uint32_t) k[6] << 16) +
              ((uint32_t) k[7] << 24));
        c += (k[8] + ((uint32_t) k[9] << 8) + ((uint32_t) k[10] << 16) +
              ((uint32_t) k[11] << 24));

        __jhash_mix(a, b, c);

        k += 12;
        len -= 12;
    }

    c += length;
    switch (len) {
        case 11:
            c += ((uint32_t) k[10] << 24);
        case 10:
            c += ((uint32_t) k[9] << 16);
        case 9:
            c += ((uint32_t) k[8] << 8);
        case 8:
            b += ((uint32_t) k[7] << 24);
        case 7:
            b += ((uint32_t) k[6] << 16);
        case 6:
            b += ((uint32_t) k[5] << 8);
        case 5:
            b += k[4];
        case 4:
            a += ((uint32_t) k[3] << 24);
        case 3:
            a += ((uint32_t) k[2] << 16);
        case 2:
            a += ((uint32_t) k[1] << 8);
        case 1:
            a += k[0];
    };

    __jhash_mix(a, b, c);
    return c;
}


