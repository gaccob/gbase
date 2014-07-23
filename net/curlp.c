#include <assert.h>
#include <stddef.h>
#include "core/os_def.h"
#include "base/slist.h"
#include "base/hash.h"

#include "curlp.h"

typedef struct curl_pool_t {
    int size;
    int running;
    CURLM* mhandle;
    slist_t* free_list;
    hash_t* clients;
} curlp_t;

static uint32_t
_curlp_hash(const void* data) {
    curlc_t* cc = (curlc_t*)(data);
    return (ptrdiff_t)(curlc_handle(cc));
}

static int
_curlp_cmp(const void* data1, const void* data2) {
    CURL* c1 = curlc_handle((curlc_t*)data1);
    CURL* c2 = curlc_handle((curlc_t*)data2);
    return (ptrdiff_t)c1 - (ptrdiff_t)c2;
}

static void
_curl_loop_release(void* data, void* args) {
    curlp_t* cp = (curlp_t*)(args);
    curlc_t* cc = (curlc_t*)(data);
    if (cp && cc) {
        curl_multi_remove_handle(cp->mhandle, curlc_handle(cc));
        curlc_release(cc);
    }
}

curlp_t*
curlp_create() {
    curlc_t* cc = NULL;
    curlp_t* cp = (curlp_t*)MALLOC(sizeof(*cp));
    if (!cp) goto CURL_FAIL;
    memset(cp, 0, sizeof(*cp));

    cp->size = CURL_POOL_DEFAULT_SIZE;
    cp->free_list = slist_create();
    if (!cp->free_list) goto CURL_FAIL1;

    cp->clients = hash_create(_curlp_hash, _curlp_cmp, cp->size * 13);
    if (!cp->clients) goto CURL_FAIL2;

    cp->mhandle = curl_multi_init();
    if (!cp->mhandle) goto CURL_FAIL3;

    // pre allocate client
    for (int i = 0; i < cp->size; ++ i) {
        cc = curlc_create();
        if (cc) {
            slist_push_front(cp->free_list, cc);
            hash_insert(cp->clients, cc);
        }
    }
    return cp;

CURL_FAIL3:
    hash_release(cp->clients);
CURL_FAIL2:
    slist_release(cp->free_list);
CURL_FAIL1:
    FREE(cp);
CURL_FAIL:
    return NULL;
}

static curlc_t*
_curlp_client_alloc(curlp_t* cp) {
    if (!cp || !cp->free_list)
        return NULL;
    if (slist_size(cp->free_list) > 0) {
        curlc_t* cc = slist_pop_front(cp->free_list);
        return cc;
    }
    cp->size ++;
    curlc_t* cc = curlc_create();
    if (cc) {
        hash_insert(cp->clients, cc);
    }
    return cc;
}

static void
_curlp_client_gc(curlp_t* cp, curlc_t* cc) {
    if (cp && cc && cp->free_list) {
        curl_multi_remove_handle(cp->mhandle, curlc_handle(cc));
        curlc_finish(cc);
        slist_push_front(cp->free_list, cc);
    }
}

void
curlp_release(curlp_t* cp) {
    if (cp) {
        if (cp->free_list) {
            slist_release(cp->free_list);
        }
        if (cp->clients) {
            hash_loop(cp->clients, _curl_loop_release, cp);
            hash_release(cp->clients);
        }
        if (cp->mhandle) {
            curl_multi_cleanup(cp->mhandle);
            cp->mhandle = NULL;
        }
        FREE(cp);
    }
}

int
curlp_add_get(curlp_t* cp, const char* req, curl_cb_func cb,
                  void* args, const char* cookie) {
    if (!cp || !req || !cb)
        return -1;
    curlc_t* cc = _curlp_client_alloc(cp);
    if (!cc)
        return -1;
    int ret = curlc_init_get(cc, req, cb, args, cookie);
    if (ret < 0) {
        _curlp_client_gc(cp, cc);
        return -1;
    }
    CURLMcode mret = curl_multi_add_handle(cp->mhandle, curlc_handle(cc));
    if (mret != CURLM_OK) {
        _curlp_client_gc(cp, cc);
        return -1;
    }
    cp->running ++;
    return 0;
}

int
curlp_add_post(curlp_t* cp, const char* req, const char* post,
                   size_t post_len, curl_cb_func cb, void* args,
                   const char* cookie) {
    if (!cp || !req || !cb)
        return -1;
    curlc_t* cc = _curlp_client_alloc(cp);
    if (!cc)
        return -1;
    int ret = curlc_init_post(cc, req, post, post_len, cb, args, cookie);
    if (ret < 0) {
        _curlp_client_gc(cp, cc);
        return -1;
    }
    CURLMcode mret = curl_multi_add_handle(cp->mhandle, curlc_handle(cc));
    if (mret != CURLM_OK) {
        _curlp_client_gc(cp, cc);
        return -1;
    }
    cp->running ++;
    return 0;
}

void
curlp_poll(curlp_t* cp) {
    if (!cp || !cp->mhandle)
        return;
    CURLMsg* msg = NULL;
    int num = 0;
    if (CURLM_OK == curl_multi_perform(cp->mhandle, &num)) {
        while ((msg = curl_multi_info_read(cp->mhandle, &num)) != NULL) {
            if (CURLMSG_DONE == msg->msg) {
                curlc_t* tmp = curlc_raw_create();
                assert(tmp);
                CURL* h = msg->easy_handle;
                curlc_set_handle(tmp, h);
                curlc_t* cc = hash_find(cp->clients, tmp);
                if (cc) {
                    curlc_set_ret(cc, msg->data.result);
                    curlc_on_res(cc);
                    _curlp_client_gc(cp, cc);
                    cp->running --;
                }
                curlc_raw_release(tmp);
            }
        }
    }
}

inline int
curlp_running_count(curlp_t* cp) {
    return cp ? cp->running : 0;
}

