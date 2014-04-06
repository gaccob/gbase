#include "core/os_def.h"
#include "net/curl_client.h"
#include "net/curl_pool.h"

typedef struct curl_client_t {
    CURL* handle;
    CURLcode err;
    char req[CURL_MAX_REQUEST_LEN];
    char res[CURL_MAX_RESPONSE_LEN];
    CURL_CALLBACK cb;
    void* args;
} curl_client_t;

curl_client_t*
curl_client_create() {
    curl_client_t* cc = (curl_client_t*)MALLOC(sizeof(*cc));
    if (!cc) { return NULL; }
    memset(cc, 0, sizeof(*cc));
    cc->handle = curl_easy_init();
    if (!cc->handle) {
        FREE(cc);
        return NULL;
    }
    curl_easy_setopt(cc->handle, CURLOPT_CONNECTTIMEOUT_MS, CURL_CONNECT_TIMEOUT_MS);
    curl_easy_setopt(cc->handle, CURLOPT_TIMEOUT_MS, CURL_TIMEOUT_MS);
    curl_easy_setopt(cc->handle, CURLOPT_VERBOSE, 0);
    curl_easy_setopt(cc->handle, CURLOPT_DNS_CACHE_TIMEOUT, 0);
    curl_easy_setopt(cc->handle, CURLOPT_FORBID_REUSE, 0);
    return cc;
}

// only alloc memory, no curl init
curl_client_t*
curl_client_raw_create() {
    curl_client_t* cc = (curl_client_t*)MALLOC(sizeof(*cc));
    if (cc) {
        memset(cc, 0, sizeof(*cc));
    }
    return cc;
}

void
curl_client_raw_release(curl_client_t* cc) {
    FREE(cc);
}

void
curl_client_release(curl_client_t* cc) {
    if (cc) {
        if (cc->handle) {
            curl_easy_cleanup(cc->handle);
            cc->handle = NULL;
        }
        FREE(cc);
    }
}

int32_t
curl_client_init_get(curl_client_t* cc, const char* req,
                     CURL_CALLBACK cb, void* args, const char* cookie) {
    if (!cc || !cc->handle) {
        return -1;
    }
    snprintf(cc->req, sizeof(cc->req), "%s", req);
    cc->cb = cb;
    cc->args = args;
    curl_easy_setopt(cc->handle, CURLOPT_URL, cc->req);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEFUNCTION, &curl_client_write_cb);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEDATA, cc);
    curl_easy_setopt(cc->handle, CURLOPT_POST, 0);
    if (cookie) {
        curl_easy_setopt(cc->handle, CURLOPT_COOKIE, cookie);
    }
    return 0;
}

int32_t
curl_client_init_post(curl_client_t* cc, const char* req,
                      const char* post, size_t post_size,
                      CURL_CALLBACK cb, void* args, const char* cookie) {
    if (!cc || !cc->handle) {
        return -1;
    }
    snprintf(cc->req, sizeof(cc->req), "%s", req);
    cc->cb = cb;
    cc->args = args;
    curl_easy_setopt(cc->handle, CURLOPT_URL, cc->req);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEFUNCTION, &curl_client_write_cb);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEDATA, cc);
    curl_easy_setopt(cc->handle, CURLOPT_POST, 1);
    curl_easy_setopt(cc->handle, CURLOPT_POSTFIELDSIZE, post_size);
    curl_easy_setopt(cc->handle, CURLOPT_COPYPOSTFIELDS, post);
    // support https
    curl_easy_setopt(cc->handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(cc->handle, CURLOPT_SSL_VERIFYHOST, 0L);
    if (cookie) {
        curl_easy_setopt(cc->handle, CURLOPT_COOKIE, cookie);
    }
    return 0;
}

void
curl_client_finish(curl_client_t* cc) {
    if (cc) {
        cc->req[0] = 0;
        cc->res[0] = 0;
        cc->cb = NULL;
        cc->args = NULL;
    }
}

inline void
curl_client_on_res(curl_client_t* cc) {
    if (cc && cc->cb) {
        (*cc->cb)(cc, cc->args);
    }
}

inline void
curl_client_append_res(curl_client_t* cc, char* data) {
    if (cc && data) {
        size_t nres = strnlen(cc->res, sizeof(cc->res));
        snprintf(cc->res + nres, sizeof(cc->res) - nres, "%s", data);
    }
}

inline CURL*
curl_client_handle(curl_client_t* cc) {
    return cc ? cc->handle : NULL;
}

inline void
curl_client_set_handle(curl_client_t* cc, CURL* c) {
    if (cc) { cc->handle = c; }
}

inline const char*
curl_client_res(curl_client_t* cc) {
    return cc ? cc->res : NULL;
}

inline const char*
curl_client_req(curl_client_t* cc) {
    return cc ? cc->req : NULL;
}

inline void*
curl_client_cb_args(curl_client_t* cc) {
    return cc ? cc->cb : NULL;
}

inline CURLcode
curl_client_ret(curl_client_t* cc) {
    return cc ? cc->err : -1;
}
inline void
curl_client_set_ret(curl_client_t* cc, CURLcode c) {
    if (cc) { cc->err = c; }
}

inline const char*
curl_client_err_str(curl_client_t* cc) {
    return cc ? curl_easy_strerror(cc->err) : NULL;
}

size_t
curl_client_write_cb(char* buffer, size_t sz, size_t nitems, void* userp) {
    curl_client_t* cc = (curl_client_t*)(userp);
    if (cc) {
        curl_client_append_res(cc, buffer);
    }
    return sz * nitems;
}

