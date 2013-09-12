#include <assert.h>
#include "tea.h"

#define TEA_LOOP 8
#define TEA_DELTA 0x9e3779b9

// src, dest: 8 bytes, key: 16 bytes
void _tea_encrypt_unit(const char* src, char* dest, const char* key)
{
    uint32_t sum = 0;
    uint32_t n = TEA_LOOP;
    uint32_t src_0 = *(uint32_t*)&src[0];
    uint32_t src_1 = *(uint32_t*)&src[4];
    const uint32_t* key_int = (const uint32_t*)key;
    while (n-- > 0) {
        sum += TEA_DELTA;
        src_0 += ((src_1 << 4) + key_int[0]) ^ (src_1 + sum) ^ ((src_1 >> 5) + key_int[1]);
        src_1 += ((src_0 << 4) + key_int[2]) ^ (src_0 + sum) ^ ((src_0 >> 5) + key_int[3]);
    }

    *(uint32_t*)(&dest[0]) = src_0;
    *(uint32_t*)(&dest[4]) = src_1;
}

// src, dest: 8bytes, key: 16 bytes
void _tea_decrypt_unit(const char* src, char* dest, const char* key)
{
    uint32_t n = TEA_LOOP;
    uint32_t sum = (TEA_DELTA  << ILOG2(TEA_LOOP));
    uint32_t src_0 = *(uint32_t*)&src[0];
    uint32_t src_1 = *(uint32_t*)&src[4];
    const uint32_t* key_int = (const uint32_t*)key;
    while (n-- > 0) {
        src_1 -= ((src_0 << 4) + key_int[2]) ^ (src_0 + sum) ^ ((src_0 >> 5) + key_int[3]);
        src_0 -= ((src_1 << 4) + key_int[0]) ^ (src_1 + sum) ^ ((src_1 >> 5) + key_int[1]);
        sum -= TEA_DELTA;
    }
    *(uint32_t*)(&dest[0]) = src_0;
    *(uint32_t*)(&dest[4]) = src_1;
}


int32_t tea_encrypt(const char* src, size_t src_len, int32_t key,
                    char* dst, size_t* dst_len)
{
    uint64_t align_add;
    uint32_t src_floor, src_mod, i;
    char key_str[16];
    assert(src && dst && dst_len);

    // align by 8
    align_add = 0;
    src_floor = src_len / 8 * 8;
    src_mod = src_len - src_floor;
    if(src_mod > 0) {
        memcpy((char*)&align_add, &src[src_floor], src_mod);
    }

    // check dst len
    if(*dst_len < src_mod + src_len + 8)
        return -1;

    // encrypt: 8bytes head(pad) + tea encrypt
    *(uint32_t*)dst = (src_mod > 0 ? 8 - src_mod : 0);

    // gen 16 bytes key
    *(uint32_t*)key_str = key;
    *(uint32_t*)(key_str + 4) = (key ^ (key >> 1));
    *(uint32_t*)(key_str + 8) = (key ^ (key >> 2));
    *(uint32_t*)(key_str + 12) = (key ^ (key >> 3));

    for(i=0; i<src_floor; i+=8)
        _tea_encrypt_unit(src + i, dst + i + 8, key_str);
    if(align_add > 0) {
        _tea_encrypt_unit((char*)&align_add, dst + src_floor + 8, key_str);
        *dst_len = src_floor + 8 + 8;
    } else {
        *dst_len = src_floor + 8;
    }

    return 0;
}

int32_t tea_descrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len)
{
    char key_str[16];
    uint32_t pad;
    size_t i;

    assert(src && dst && dst_len);
    assert((src_len % 8 == 0) && (src_len > 8));

    if(*dst_len < src_len - 8)
        return -1;

    // gen 16 bytes key
    *(uint32_t*)key_str = key;
    *(uint32_t*)(key_str + 4) = (key ^ (key >> 1));
    *(uint32_t*)(key_str + 8) = (key ^ (key >> 2));
    *(uint32_t*)(key_str + 12) = (key ^ (key >> 3));

    // head
    pad = *(uint32_t*)src;

    // descrypt by unit
    for(i=8; i<src_len; i+=8)
        _tea_decrypt_unit(src + i, dst + i - 8, key_str);
    *dst_len = src_len - 8 - pad;

    return 0;
}

