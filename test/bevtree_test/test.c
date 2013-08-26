#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <ds/bevtree.h>

#define TEST_LOOP 3 

typedef struct input_t {
    int32_t ratio30;
    int32_t ratio31;
} input_t;

int32_t condition_300_cb(void* input)
{
    input_t* i = (input_t*)(input);
    if (i->ratio30 < 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}
int32_t condition_301_cb(void* input)
{
    input_t* i = (input_t*)(input);
    if (i->ratio30 >= 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}
int32_t condition_310_cb(void* input)
{
    input_t* i = (input_t*)(input);
    if (i->ratio31 < 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}
int32_t condition_311_cb(void* input)
{
    input_t* i = (input_t*)(input);
    if (i->ratio31 >= 50)
        return BVT_SUCCESS;
    return BVT_ERROR;
}

int32_t action_400_cb(void* input)
{
    printf("400: 去球场\n");
    return BVT_SUCCESS;
}
int32_t action_401_cb(void* input)
{
    printf("401: 打球\n");
    return BVT_SUCCESS;
}
int32_t action_402_cb(void* input)
{
    printf("402: 和妹纸吃饭\n");
    return BVT_SUCCESS;
}
int32_t action_403_cb(void* input)
{
    printf("403: 回家滚床单\n");
    return BVT_SUCCESS;
}
int32_t action_404_cb(void* input)
{
    printf("404: 回去撸一把\n");
    return BVT_SUCCESS;
}

void test_bvt(const char* cfg)
{
    struct bvt_t* bvt = bvt_load_gliffy(cfg);
    if (!bvt)
    {
        printf("bvt init fail\n");
        return;
    }
    bvt_debug(bvt);

    int32_t ret = -1;
    ret = bvt_register_callback(bvt, action_400_cb, 400);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_401_cb, 401);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_402_cb, 402);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_403_cb, 403);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, action_404_cb, 404);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_300_cb, 300);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_301_cb, 301);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_310_cb, 310);
    assert(ret == BVT_SUCCESS);
    ret = bvt_register_callback(bvt, condition_311_cb, 311);
    assert(ret == BVT_SUCCESS);

    int32_t loop = 0;
    while (loop ++ < TEST_LOOP) {
        input_t i;
        i.ratio30 = rand() % 100;
        i.ratio31 = rand() % 100;
        ret = bvt_run(bvt, (void*)&i);
        assert(BVT_SUCCESS == ret);
        printf("============\n\n");
    }

    bvt_release(bvt);
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        printf("usage: ./test cfg_file_path\n");
        return -1;
    }
    srand(time(NULL));
    test_bvt(argv[1]);
    return 0;
}
