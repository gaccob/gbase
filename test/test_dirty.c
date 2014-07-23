#include <assert.h>
#include <sys/time.h>
#include "logic/dirty.h"
#include "util/util_time.h"

#define DIRTY_LOOP 10

#define DIRTY_CFG_FILE "dirty_cfg_words"
#define DIRTY_TEST_FILE "dirty_test_words"

int
test_dirty() {
    dirty_ctx_t* ctx;
    char logtime[64];
    struct timeval tv;

    ctx = dirty_create(DIRTY_CFG_FILE);
    assert(ctx);
    printf("dirty check init success\n");

    char source[1024 * 8];
    memset(source, 0, sizeof(source));
    FILE* fp = fopen(DIRTY_TEST_FILE, "r");
    if (!fp) {
        return -1;
    }
    fgets(source, sizeof(source), fp);
    printf("source len=%d\n", (int)strlen(source));

    gettimeofday(&tv, NULL);
    util_timestamp(&tv, logtime, sizeof(logtime));
    printf("start %s\n", logtime);

    for (int i = 0; i < DIRTY_LOOP; ++ i) {
        char tmp[1024 * 8];
        snprintf(tmp, sizeof(tmp), "%s", source);
        int ret = dirty_replace(ctx, tmp, strlen(tmp));
        assert(ret == 0);
    }

    gettimeofday(&tv, NULL);
    util_timestamp(&tv, logtime, sizeof(logtime));
    printf("end %s\n", logtime);

    dirty_release(ctx);
    printf("dirty check finish success\n");
    return 0;
}

