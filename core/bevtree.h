#ifndef BEVTREE_H_
#define BEVTREE_H_

//
// a behavior tree relies on gliffy diagram
// reference: http://www.gaccob.com/?p=1226
//

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

enum
{
    BVT_SUCCESS = 0,
    BVT_BACKTRACK = 100,
    BVT_ERROR = -100,
    BVT_EXECUTE_ERROR = -101,
    BVT_SELECTOR_ERROR = -102,
    BVT_CONDITION_ERROR = -103,
    BVT_ACTION_ERROR = -104,
    BVT_SEQUENCE_ERROR = -105,
    BVT_PARALLEL_ERROR = -106,
    BVT_CALLBACK_DUPLICATED = -107,
    BVT_CONFIG_ERROR = -108,
    BVT_CONFIG_NAME_ERROR = -109,
    BVT_CONFIG_TYPE_ERROR = -110,
    BVT_GLIFFY_ERROR = -111,
};


// return BVT error code
typedef int32_t (*bvt_callback)(void* input);

struct bvt_t;
struct bvt_t* bvt_load_gliffy(const char* gliffy_path);
int32_t bvt_register_callback(struct bvt_t* b, bvt_callback cb, int32_t id);
int32_t bvt_run(struct bvt_t* b, void* input);
void bvt_release(struct bvt_t* b);
void bvt_debug(struct bvt_t* b);

#ifdef __cplusplus
}
#endif

#endif
