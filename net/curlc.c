#include "core/os_def.h"
#include "curlc.h"

struct curl_client_t {
    CURL* handle;
    CURLcode err;
    char req[CURL_MAX_REQUEST_LEN];
    char res[CURL_MAX_RESPONSE_LEN];
    curl_cb_func cb;
    void* args;
};

curlc_t*
curlc_create() {
    curlc_t* cc = (curlc_t*)MALLOC(sizeof(*cc));
    if (!cc)
        return NULL;
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
curlc_t*
curlc_raw_create() {
    curlc_t* cc = (curlc_t*)MALLOC(sizeof(*cc));
    if (cc)
        memset(cc, 0, sizeof(*cc));
    return cc;
}

void
curlc_raw_release(curlc_t* cc) {
    FREE(cc);
}

void
curlc_release(curlc_t* cc) {
    if (cc) {
        if (cc->handle) {
            curl_easy_cleanup(cc->handle);
            cc->handle = NULL;
        }
        FREE(cc);
    }
}

int32_t
curlc_init_get(curlc_t* cc, const char* req, curl_cb_func cb,
               void* args, const char* cookie) {
    if (!cc || !cc->handle) {
        return -1;
    }
    snprintf(cc->req, sizeof(cc->req), "%s", req);
    cc->cb = cb;
    cc->args = args;
    curl_easy_setopt(cc->handle, CURLOPT_URL, cc->req);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEFUNCTION, &curlc_write_cb);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEDATA, cc);
    curl_easy_setopt(cc->handle, CURLOPT_POST, 0);
    if (cookie) {
        curl_easy_setopt(cc->handle, CURLOPT_COOKIE, cookie);
    }
    return 0;
}

int32_t
curlc_init_post(curlc_t* cc, const char* req, const char* post, size_t post_size,
                curl_cb_func cb, void* args, const char* cookie) {
    if (!cc || !cc->handle) {
        return -1;
    }
    snprintf(cc->req, sizeof(cc->req), "%s", req);
    cc->cb = cb;
    cc->args = args;
    curl_easy_setopt(cc->handle, CURLOPT_URL, cc->req);
    curl_easy_setopt(cc->handle, CURLOPT_WRITEFUNCTION, &curlc_write_cb);
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
curlc_finish(curlc_t* cc) {
    if (cc) {
        cc->req[0] = 0;
        cc->res[0] = 0;
        cc->cb = NULL;
        cc->args = NULL;
    }
}

inline void
curlc_on_res(curlc_t* cc) {
    if (cc && cc->cb) {
        (*cc->cb)(cc, cc->args);
    }
}

inline void
curlc_append_res(curlc_t* cc, char* data) {
    if (cc && data) {
        size_t nres = strlen(cc->res);
        snprintf(cc->res + nres, sizeof(cc->res) - nres, "%s", data);
    }
}

inline CURL*
curlc_handle(curlc_t* cc) {
    return cc ? cc->handle : NULL;
}

inline void
curlc_set_handle(curlc_t* cc, CURL* c) {
    if (cc) {
        cc->handle = c;
    }
}

inline const char*
curlc_res(curlc_t* cc) {
    return cc ? cc->res : NULL;
}

inline const char*
curlc_req(curlc_t* cc) {
    return cc ? cc->req : NULL;
}

inline void*
curlc_cb_args(curlc_t* cc) {
    return cc ? cc->cb : NULL;
}

inline CURLcode
curlc_ret(curlc_t* cc) {
    return cc ? cc->err : -1;
}
inline void
curlc_set_ret(curlc_t* cc, CURLcode c) {
    if (cc) {
        cc->err = c;
    }
}

inline const char*
curlc_err_str(curlc_t* cc) {
    return cc ? curl_easy_strerror(cc->err) : NULL;
}

size_t
curlc_write_cb(char* buffer, size_t sz, size_t nitems, void* userp) {
    curlc_t* cc = (curlc_t*)(userp);
    if (cc) {
        curlc_append_res(cc, buffer);
    }
    return sz * nitems;
}

