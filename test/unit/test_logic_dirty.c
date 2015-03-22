#include <assert.h>
#include <sys/time.h>

#include "logic/dirty.h"

#define DIRTY_CFG_FILE "dirty_cfg_words"

int
test_logic_dirty(char* param) {
    dirty_ctx_t* ctx = dirty_create(DIRTY_CFG_FILE);
    if (!ctx) {
        fprintf(stderr, "create dirty context fail\n");
        return -1;
    }

    char* source = param ? param : "test dirty words fucking";
    int size = strlen(source) + 1;
    char result[size];
    snprintf(result, sizeof(result), "%s", source);
    int ret = dirty_replace(ctx, result, size);
    if (ret < 0) {
        fprintf(stderr, "dirty replace fail:%d\n", ret);
        dirty_release(ctx);
        return ret;
    }
    printf("[%s] -> [%s]\n", source, result);

    dirty_release(ctx);
    return 0;
}

