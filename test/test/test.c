#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "os/os_def.h"
#include "os/util.h"

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


int main()
{
    test_base64();
    return 0;
}

