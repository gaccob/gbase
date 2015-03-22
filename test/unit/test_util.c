#include "util/base64.h"
#include "util/sha1.h"
#include "util/random.h"
#include "util/encode.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_base64(char* param) {
    const char* const src = "Man is distinguished, not only by his reason,"
        " but by this singular passion from other animals, which is a lust"
        " of the mind, that by a perseverance of delight in the continued"
        " and indefatigable generation of knowledge, exceeds the short"
        " vehemence of any carnal pleasure.";
    char dst[1024], src2[1024];
    memset(dst, 0, sizeof(dst));
    memset(src2, 0, sizeof(src2));
    if (base64_encode(dst, src, strlen(src)) < 0) {
        fprintf(stderr, "base64 encode fail\n");
        return -1;
    }
    if (base64_decode(src2, dst, strlen(dst)) < 0) {
        fprintf(stderr, "base64 decode fail\n");
        return -1;
    }
    if (strcmp(src2, src)) {
        fprintf(stderr, "base64 decode: %s\n", src2);
        return -1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static int
_test_util_wscode(const char* req, const char* res) {
    char key[64], sha[128], base64[128];
    memset(sha, 0, sizeof(sha));
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", req);
    sha1(sha, key, strlen(key) * 8);
    if (base64_encode(base64, sha, strlen(sha)) < 0) {
        fprintf(stderr, "base64 encode fail\n");
        return -1;
    }
    printf("wscode [%s]->[%s]\n", req, base64);
    if (res && strcmp(base64, res)) {
        fprintf(stderr, "fail, expect [%s]\n", res);
        return -1;
    }
    return 0;
}

int
test_util_wscode(char* param) {
    char* req = "2SCVXUeP9cTjV+0mWB8J6A==";
    char* res = "mLDKNeBNWz6T9SxU+o0Fy/HgeSw=";
    int ret = 0;
    ret |= _test_util_wscode(req, res);
    if (param) {
        ret |= _test_util_wscode(param, NULL);
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_random(char* param) {
    int32_t i;
    uint32_t r;
    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < 16; ++ i) {
        r = rand_gen();
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_shuffle(char* param) {
    int size = param ? atoi(param) : 52;
	int i, cards[size];
    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < size; ++ i)
        cards[i] = i;
    rand_shuffle(cards, size);
    for (i = 0; i < size; ++ i)
        printf("%d ", cards[i]);
    printf("\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_unicode(char* param) {
    int unicode = 0;
    char* utf8 = "这是一个测试串";
    char** src = &utf8;
    while (0 == utf8_unicode(src, &unicode)) {
        printf("%d ", unicode);
    }
    printf("\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

