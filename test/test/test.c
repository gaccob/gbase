#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "core/os_def.h"
#include "util/base64.h"
#include "util/sha1.h"
#include "util/random.h"
#include "core/conhash.h"

#ifdef OS_LINUX
#include "core/coroutine.h"

#define CRT_TEST_STACK_SIZE 4096
void crt_func(struct crt_t* c, void* arg)
{
    int32_t index = *(int32_t*)(arg);
    int32_t tick;
    char tmp[100];
    tmp[0] = 'a';
    for (tick = 0; tick < 5; ++ tick)
    {
        printf("coroutine[%d] tick %d\n", crt_current(c), index + tick);
        char dummy;
        printf("stack size: %d\n", (int)(crt_current_stack_top(c) - &dummy));
        crt_yield(c);
    }
    printf("coroutine[%d] finish\n", crt_current(c));
}

int32_t test_coroutine()
{
    int i, j, m, n;
    int c1, c2, c3, c4;
    struct crt_t* c = crt_init(CRT_TEST_STACK_SIZE);
    assert(c);

    i = 10;
    c1 = crt_new(c, crt_func, &i);
    assert(c1 >= 0);
    j = 100;
    c2 = crt_new(c, crt_func, &j);
    assert(c2 >= 0);
    while (crt_status(c, c1) && crt_status(c, c2)) {
        crt_resume(c, c1);
        crt_resume(c, c2);
    }

    m = -10;
    c3 = crt_new(c, crt_func, &m);
    assert(c3 >= 0);
    n = -100;
    c4 = crt_new(c, crt_func, &m);
    assert(c4 >= 0);
    while (crt_status(c, c3) && crt_status(c, c4)) {
        crt_resume(c, c3);
        crt_resume(c, c4);
    }

    crt_release(c);
    return 0;
}
#endif

int32_t test_base64()
{
    const char* const src = "Man is distinguished, not only by his reason, but by this singular passion from other animals, which is a lust of the mind, that by a perseverance of delight in the continued and indefatigable generation of knowledge, exceeds the short vehemence of any carnal pleasure.";
    char dst[1024], src2[1024];
    memset(dst, 0, sizeof(dst));
    memset(src2, 0, sizeof(src2));
    printf("source=%d: %s\n", (int)strlen(src), src);
    if (base64_encode(dst, src, strlen(src)) < 0) {
        printf("base64 encode fail\n");
        return -1;
    }
    printf("base64 encode=%d: %s\n", (int)strlen(dst), dst);
    if (base64_decode(src2, dst, strlen(dst)) < 0) {
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
    base64_encode(base64, sha, strlen(sha));
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
uint32_t conhash_key_hash(const void* key)
{
    const struct key_t* k = (const struct key_t*)key;
    return hash_jhash(k->key, strlen(k->key));
}
uint32_t conhash_node_hash(const void* node)
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
    for (i=0; i<4; i++) {
        snprintf(node[i].name, sizeof(node[i].name), "node_%d", i);
        ret = conhash_add_node(ch, &node[i]);
        assert(0 == ret);
    }
    for (i=0; i<10; i++) {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_erase_node(ch, &node[0]);
    for (i=0; i<10; i++) {
        struct key_t k;
        snprintf(k.key, sizeof(k.key), "key_%d", i);
        n = conhash_node(ch, &k);
        assert(n);
        printf("%s: %s\n", k.key, n->name);
    }
    printf("============\n");
    conhash_add_node(ch, &node[0]);
    for (i=0; i<10; i++) {
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

void test_random()
{
    int32_t i;
    uint32_t r;
    rand_seed(time(NULL));
    for (i = 0; i < 1000; ++ i) {
        r = rand_gen();
        printf("%u\t", r);
        if (i % 8 == 0) printf("\n");
    }
}

int main()
{
    // test_base64();
    // test_ws();
    // test_conhash();

#ifdef OS_LINUX
    // test_coroutine();
#endif

    test_random();

    return 0;
}

