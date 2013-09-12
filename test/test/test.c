#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core/os_def.h"
#include "core/util.h"
#include "ds/sha1.h"
#include "ds/conhash.h"

int32_t test_base64()
{
    const char* const src = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    char dst[1024], src2[1024];
    memset(dst, 0, sizeof(dst));
    memset(src2, 0, sizeof(src2));
    printf("source=%d: %s\n", (int)strlen(src), src);
    if(util_base64_encode(dst, src, strlen(src)) < 0)
    {
        printf("base64 encode fail\n");
        return -1;
    }
    printf("base64 encode=%d: %s\n", (int)strlen(dst), dst);
    if(util_base64_decode(src2, dst, strlen(dst)) < 0)
    {
        printf("base64 decode fail\n");
        return -1;
    }
    printf("base64 decode=%d: %s\n", (int)strlen(src2), src2);
    return 0;
}


int32_t test_ws()
{
    char* req = "2SCVXUeP9cTjV+0mWB8J6A=="; //"dGhlIHNhbXBsZSBub25jZQ==";
    char key[64], sha[128], base64[128];
    memset(sha, 0, sizeof(sha));
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", req);
    sha1(sha, key, strlen(key) * 8);
    util_base64_encode(base64, sha, strlen(sha));
    printf("%s\n", base64);
    return 0;
}

#if !defined OS_WIN
struct key_t
{
    char key[32];
};
struct node_t
{
    char name[64];
};
int32_t conhash_key_hash(const void* key)
{
    const struct key_t* k = (const struct key_t*)key;
    return hash_jhash(k->key, strlen(k->key));
}
int32_t conhash_node_hash(const void* node)
{
    const struct node_t* n = (const struct node_t*)node;
    return hash_jhash(n->name, strlen(n->name));
}
int32_t test_conhash()
{
    struct conhash_t* ch = conhash_init(conhash_key_hash, conhash_node_hash);
    assert(ch);
    struct node_t node[10];
    struct node_t* n;
    int32_t i, ret;
    for (i=0; i<4; i++)
    {
        snprintf(node[i].name, sizeof(node[i].name), "node_%d", i);
        ret = conhash_add_node(ch, &node[i]);
        assert(0 == ret);
    }
    for (i=0; i<10; i++)
    {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_erase_node(ch, &node[0]);
    for (i=0; i<10; i++)
    {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_add_node(ch, &node[0]);
    for (i=0; i<10; i++)
    {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    conhash_release(ch);
    return 0;
}
#endif

int main()
{
    //test_base64();
    //test_ws();
    //test_conhash();
    return 0;
}

