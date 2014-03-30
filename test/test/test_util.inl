#include "util/base64.h"
#include "util/sha1.h"
#include "util/random.h"
#include "util/encode.h"

int32_t
test_base64() {
    const char* const src = "Man is distinguished, not only by his reason,"
        " but by this singular passion from other animals, which is a lust"
        " of the mind, that by a perseverance of delight in the continued"
        " and indefatigable generation of knowledge, exceeds the short"
        " vehemence of any carnal pleasure.";
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

int32_t
test_wscode() {
    char* req = "2SCVXUeP9cTjV+0mWB8J6A=="; //"dGhlIHNhbXBsZSBub25jZQ==";
    char key[64], sha[128], base64[128];
    memset(sha, 0, sizeof(sha));
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", req);
    sha1(sha, key, strlen(key) * 8);
    base64_encode(base64, sha, strlen(sha));
    printf("%s\n", base64);
    return 0;
}

void
test_random() {
    int32_t i;
    uint32_t r;
    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < 1000; ++ i) {
        r = rand_gen();
        printf("%u\t", r);
        if (i % 8 == 0) printf("\n");
    }
    printf("\n\n");
}

void
test_shuffle() {
	const size_t sz = 52;
	int32_t cards[52];
    size_t i;

    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < sz; ++ i)
        cards[i] = (int32_t)i;
    rand_shuffle(cards, sz);
    for (i = 0; i < sz; ++ i)
        printf("%d ", cards[i]);
    printf("\n");
}

void
test_unicode() {
    int unicode = 0;
    char* utf8 = "张伟业";
    char** src = &utf8;
    while (0 == get_unicode(src, &unicode)) {
        printf("%d ", unicode);
    }
    printf("\n");
}

