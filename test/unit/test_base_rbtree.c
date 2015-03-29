#include <assert.h>

#include "base/rbtree.h"

static int
_cmp_func(void* data1, void* data2) {
    return *(int*)data1 - *(int*)data2;
}

static void
_loop_func(void* data) {
    printf("%d ", *(int*)data);
}

int
test_base_rbtree(char* param) {

    struct rbtree_t* tree = rbtree_create(_cmp_func);
    if (!tree) {
        fprintf(stderr, "rbtree create fail\n");
        return -1;
    }

    printf("\n");

    int loop = param ? atoi(param) : 32;
    int val[loop];
    for (int i = 0; i < loop; ++ i) {
        // avoid duplicate
        val[i] = (rand() % loop) * loop + i;
        int ret = rbtree_insert(tree, &val[i]);
        if (ret < 0) {
            fprintf(stderr, "insert val=%d ret=%d\n", val[i], ret);
            rbtree_release(tree);
            return -1;
        }

        printf("insert %d: === ", val[i]);
        rbtree_loop(tree, _loop_func);
        printf(" ===\n");
    }

    for (int i = 0; i < loop; ++ i) {
        int ret = rbtree_insert(tree, &val[i]);
        if (ret == 0) {
            fprintf(stderr, "insert duplicate val=%d ret=%d\n", val[i], ret);
            rbtree_release(tree);
            return -1;
        }
    }

    for (int i = 0; i < loop; ++ i) {
        if (val[i] >= 0) {
            int* res = rbtree_find(tree, &val[i]);
            if (*res != val[i]) {
                fprintf(stderr, "find val=%d fail\n", val[i]);
                rbtree_release(tree);
                return -1;
            }
        }
    }

    printf("\n");

    for (int i = 0; i < loop; ++ i) {
        if (val[i] >= 0) {
            int* res = rbtree_delete(tree, &val[i]);
            if (*res != val[i]) {
                fprintf(stderr, "erase val=%d fail\n", val[i]);
                rbtree_release(tree);
                return -1;
            }
            printf("erase %d: === ", val[i]);
            rbtree_loop(tree, _loop_func);
            printf(" ===\n");
        }
    }

    printf("\n");
    rbtree_release(tree);
    return 0;
}

