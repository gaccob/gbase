#include "core/rbtree.h"
#include <assert.h>

#define LOOP 32

int cmp_func(void* data1, void* data2)
{
    return *(int*)data1 - *(int*)data2;
}

void loop_func(void* data)
{
    printf("%d ", *(int*)data);
}

int main()
{
    int val[LOOP];
    int i, ret;
    int* res;
    struct rbtree_t* tree;
    tree = rbtree_init(cmp_func);
    assert(tree);

    for(i = 0; i < LOOP; i++)
    {
        val[i] = rand() % LOOP * 3;
        ret = rbtree_insert(tree, &val[i]);
        printf("insert val=%d ret=%d\n", val[i], ret);
        if(ret < 0)    val[i] = -1;
    }

    for(i = 0; i < LOOP; i++)
    {
        if(val[i] >= 0)
        {
            res = rbtree_find(tree, &val[i]);
            assert(*res == val[i]);
        }
    }

    printf("=== data ");
    rbtree_loop(tree, loop_func);
    printf(" ===\n\n");

    for(i = 0; i < LOOP; i++)
    {
        if(val[i] >= 0)
        {
            res = rbtree_delete(tree, &val[i]);
            assert(*res == val[i]);
            printf("erase %d: === ", val[i]);
            rbtree_loop(tree, loop_func);
            printf(" ===\n");
        }
    }

    rbtree_release(tree);
    getchar();
    return 0;
}

