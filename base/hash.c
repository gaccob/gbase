#include <assert.h>
#include <string.h>
#include "hash.h"

typedef struct node_t {
    void* m_data;
    struct node_t* m_next;
} node_t;

struct hash_t {
    int m_size;
    int m_count;
    hash_func m_hash_func;
    hash_cmp_func m_cmp_func;
    node_t** m_table;
};

hash_t*
hash_create(hash_func hash, hash_cmp_func cmp, int hint_size) {
    if (!hash || !cmp || hint_size <= 0) {
        return NULL;
    }
    hash_t* htable = (hash_t*)MALLOC(sizeof(hash_t));
    if (!htable) {
        goto HASH_FAIL;
    }
    memset(htable, 0, sizeof(*htable));
    htable->m_size = hint_size;
    htable->m_cmp_func = cmp;
    htable->m_hash_func = hash;
    htable->m_table = (node_t**)MALLOC(sizeof(node_t*) * htable->m_size);
    if (!htable->m_table) {
        goto HASH_FAIL1;
    }
    memset(htable->m_table, 0, sizeof(node_t*) * htable->m_size);
    return htable;
HASH_FAIL1:
    FREE(htable);
HASH_FAIL:
    return NULL;
}

int
hash_release(hash_t* htable) {
    if (!htable)
        return -1;
    hash_clean(htable);
    FREE(htable->m_table);
    FREE(htable);
    return 0;
}

int
hash_clean(hash_t* htable) {
    if (!htable) {
        return -1;
    }
    for (int i = 0; i < htable->m_size; ++ i) {
        // free list node
        node_t* node = htable->m_table[i];
        while (node) {
            node_t* bak = node;
            node->m_data = 0;
            node = node->m_next;
            FREE(bak);
        }
        htable->m_table[i] = 0;
    }
    return 0;
}

int
hash_loop(hash_t* htable, hash_loop_func f, void* args) {
    if (!htable || !f) {
        return -1;
    }
    for (int i = 0; i < htable->m_size; ++ i) {
        node_t* node = htable->m_table[i];
        while (node) {
            if (node->m_data) {
                (*f)(node->m_data, args);
            }
            node = node->m_next;
        }
    }
    return 0;
}

int
hash_insert(hash_t* htable, void* data) {
    if (!htable || !data) {
        return -1;
    }
    uint32_t hash_key = htable->m_hash_func(data);
    uint32_t index = hash_key % htable->m_size;
    node_t* node = htable->m_table[index];
    node_t* prev = 0;
    while (node) {
        if (0 == htable->m_cmp_func(node->m_data, data)) {
            return -1;
        }
        prev = node;
        node = node->m_next;
    }
    node = (node_t*)MALLOC(sizeof(node_t));
    if (!node)
        return -1;
    node->m_data = data;
    node->m_next = 0;
    if (prev) {
        prev->m_next = node;
    } else {
        htable->m_table[index] = node;
    }
    ++ htable->m_count;
    return 0;
}

void*
hash_remove(hash_t* htable, void* data) {
    if (!htable || !data) {
        return NULL;
    }
    uint32_t hash_key = htable->m_hash_func(data);
    uint32_t index = hash_key % htable->m_size;
    node_t* node = htable->m_table[index];
    node_t* prev = 0;
    while (node) {
        if (0 == htable->m_cmp_func(node->m_data, data)) {
            if (prev) {
                prev->m_next = node->m_next;
            } else {
                htable->m_table[index] = 0;
            }
            void* ret = node->m_data;
            FREE(node);
            node = 0;
            -- htable->m_count;
            return ret;
        }
        prev = node;
        node = node->m_next;
    }
    return NULL;
}

int
hash_count(hash_t* htable) {
    return htable ? htable->m_count : -1;
}


void*
hash_find(hash_t* htable, void* data) {
    if (!htable || !data) {
        return NULL;
    }
    uint32_t hash_key = htable->m_hash_func(data);
    node_t* node = htable->m_table[hash_key % htable->m_size];
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

