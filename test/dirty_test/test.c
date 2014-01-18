#include <assert.h>
#include "logic/dirty/dirty.h"
#include "util/util_time.h"

#define MAX_LOOP 100000

int main(int argc, char** argv)
{
    struct dirty_ctx_t* ctx;
    char source[1024 * 8];
    char tmp[1024 * 8];
    char logtime[64];
    FILE* fp;
    struct timeval tv;
    int i, ret;

    if (argc != 3) {
        printf("usage: ./dirty cfg_file check_file\n");
        return 0;
    }

    ctx = dirty_init(argv[1]);
    assert(ctx);
    printf("dirty check init success\n");

    memset(source, 0, sizeof(source));
    fp = fopen(argv[2], "r");
    if (!fp) {
        return -1;
    }
    fgets(source, sizeof(source), fp);
    printf("source len=%d\n", (int)strlen(source));

/*
    printf("before replace: \n%s\n\n", source);
    dirty_replace(ctx, source, strlen(source));
    printf("after replace: \n%s\n", source);
    fclose(fp);
 */   

    util_gettimeofday(&tv, NULL);
    util_timestamp(&tv, logtime, sizeof(logtime));
    printf("start %s\n", logtime);

    for (i = 0; i < MAX_LOOP; ++ i) {
        snprintf(tmp, sizeof(tmp), "%s", source);
        ret = dirty_replace(ctx, tmp, strlen(tmp));
        assert(ret == 0);
    }

    util_gettimeofday(&tv, NULL);
    util_timestamp(&tv, logtime, sizeof(logtime));
    printf("end %s\n", logtime);

    dirty_release(ctx);
    printf("dirty check finish success\n");
    return 0;
}

