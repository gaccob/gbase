#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "os/os_def.h"
#include "os/util.h"
#include "ds/sha1.h"

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


int main()
{
    //test_base64();
    test_ws();
    return 0;
}

