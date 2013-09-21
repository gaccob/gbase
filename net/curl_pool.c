#include <assert.h>
#include <sys/time.h>
#include <stddef.h>
#include "core/os_def.h"
#include "net/curl_pool.h"
#include "net/curl_client.h"
#include "ds/slist.h"
#include "ds/hash.h"

typedef struct curl_pool_t
{
    int32_t size;
    int32_t running;
    CURLM* mhandle;
    struct slist_t* free_list;
    struct hash_t* clients;
} curl_pool_t;

uint32_t _curl_pool_hash(const void* data)
{
    struct curl_client_t* cc = (struct curl_client_t*)(data);
    return (ptrdiff_t)(curl_client_handle(cc));
}

int32_t _curl_pool_cmp(const void* data1, const void* data2)
{
    CURL* c1 = curl_client_handle((struct curl_client_t*)data1);
    CURL* c2 = curl_client_handle((struct curl_client_t*)data2);
    return (ptrdiff_t)c1 - (ptrdiff_t)c2;
}

void _curl_loop_release_client(void* data, void* args)
{
    struct curl_pool_t* cp = (struct curl_pool_t*)(args);
    struct curl_client_t* cc = (struct curl_client_t*)(data);
    if (cp && cc) {
        curl_multi_remove_handle(cp->mhandle, curl_client_handle(cc));
        curl_client_release(cc);
    }
}

struct curl_pool_t* curl_pool_init()
{
    int32_t i = 0;
    struct curl_client_t* cc = NULL;
    struct curl_pool_t* cp = (struct curl_pool_t*)MALLOC(sizeof(*cp));
    if (!cp) goto CURL_FAIL;
    memset(cp, 0, sizeof(*cp));

    cp->size = CURL_POOL_DEFAULT_SIZE;
    cp->free_list = slist_init();
    if (!cp->free_list) goto CURL_FAIL1;

    cp->clients = hash_init(_curl_pool_hash, _curl_pool_cmp, cp->size * 13);
    if (!cp->clients) goto CURL_FAIL2;

    cp->mhandle = curl_multi_init();
    if (!cp->mhandle) goto CURL_FAIL3;

    // pre allocate client
    for (i = 0; i < cp->size; ++ i)
    {
        cc = curl_client_init();
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

struct curl_client_t* _curl_pool_client_alloc(struct curl_pool_t* cp)
{
    struct curl_client_t* cc = NULL;
    if (!cp || !cp->free_list) { return NULL; }

    if (slist_count(cp->free_list) > 0) {
        cc = slist_pop_front(cp->free_list);
        return cc;
    }

    cp->size ++;
    cc = curl_client_init();
    if (cc) {
        hash_insert(cp->clients, cc);
    }
    return cc;
}

void _curl_pool_client_gc(struct curl_pool_t* cp,
                          struct curl_client_t* cc)
{
    if (cp && cc && cp->free_list) {
        curl_multi_remove_handle(cp->mhandle, curl_client_handle(cc));
        curl_client_finish_req(cc);
        slist_push_front(cp->free_list, cc);
    }
}

void curl_pool_release(struct curl_pool_t* cp)
{
    if (cp) {
        if (cp->free_list) {
            slist_release(cp->free_list);
        }
        if (cp->clients) {
            hash_loop(cp->clients, _curl_loop_release_client, cp);
            hash_release(cp->clients);
        }
        if (cp->mhandle) {
            curl_multi_cleanup(cp->mhandle);
            cp->mhandle = NULL;
        }
        FREE(cp);
    }
}

int32_t curl_pool_add_get_req(struct curl_pool_t* cp, const char* req,
                              CURL_CALLBACK cb, void* args,
                              const char* cookie)
{
    struct curl_client_t* cc = NULL;
    int32_t ret;
    CURLMcode mret;

    if (!cp || !req || !cb) return -1;
    cc = _curl_pool_client_alloc(cp);
    if (!cc) return -1;

    ret = curl_client_init_get_req(cc, req, cb, args, cookie);
    if (ret < 0) {
        _curl_pool_client_gc(cp, cc);
        return -1;
    }

    mret = curl_multi_add_handle(cp->mhandle, curl_client_handle(cc));
    if (mret != CURLM_OK) {
        _curl_pool_client_gc(cp, cc);
        return -1;
    }

    cp->running ++;
    return 0;
}

int32_t curl_pool_add_post_req(struct curl_pool_t* cp, const char* req,
                               const char* post, size_t post_len,
                               CURL_CALLBACK cb, void* args,
                               const char* cookie)
{
    struct curl_client_t* cc = NULL;
    int32_t ret;
    CURLMcode mret;

    if (!cp || !req || !cb) return -1;
    cc = _curl_pool_client_alloc(cp);
    if (!cc) return -1;

    ret = curl_client_init_post_req(cc, req, post, post_len, cb, args, cookie);
    if (ret < 0) {
        _curl_pool_client_gc(cp, cc);
        return -1;
    }

    mret = curl_multi_add_handle(cp->mhandle, curl_client_handle(cc));
    if (mret != CURLM_OK) {
        _curl_pool_client_gc(cp, cc);
        return -1;
    }

    cp->running ++;
    return 0;
}

void curl_pool_run(struct curl_pool_t* cp)
{
    int32_t num = 0;
    CURLMsg* msg = NULL;
    CURL* h = NULL;
    struct curl_client_t* cc = NULL;
    struct curl_client_t* tmp = NULL;
    if (!cp || !cp->mhandle) { return; }

    if (CURLM_OK == curl_multi_perform(cp->mhandle, &num)) {
        while ((msg = curl_multi_info_read(cp->mhandle, &num)) != NULL) {
            if (CURLMSG_DONE == msg->msg) {
                tmp = curl_client_raw_init();
                assert(tmp);
                h = msg->easy_handle;
                curl_client_set_handle(tmp, h);
                cc = hash_find(cp->clients, tmp);
                if (cc) {
                    curl_client_set_err_code(cc, msg->data.result);
                    curl_client_on_res(cc);
                    _curl_pool_client_gc(cp, cc);
                    cp->running --;
                }
                curl_client_raw_release(tmp);
            }
        }
    }
}

int32_t curl_pool_running_count(struct curl_pool_t* cp)
{
    return cp ? cp->running : 0;
}

