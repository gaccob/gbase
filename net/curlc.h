#ifndef CURLC_H_
#define CURLC_H_

#ifdef __cpluplus
extern "C" {
#endif

#include <stdint.h>
#include <curl/curl.h>

typedef struct curl_client_t curlc_t;
typedef void (*curl_cb_func)(curlc_t*, void* args);

#define CURL_MAX_REQUEST_LEN 4096
#define CURL_MAX_RESPONSE_LEN 4096

#define CURL_CONNECT_TIMEOUT_MS 3000
#define CURL_TIMEOUT_MS 3000

curlc_t* curlc_create();
void curlc_release(curlc_t*);

// only alloc memory, no curl init
curlc_t* curlc_raw_create();
void curlc_raw_release(curlc_t*);

int32_t curlc_init_get(curlc_t*, const char* req, curl_cb_func cb,
                       void* args, const char* cookie);
int32_t curlc_init_post(curlc_t*, const char* req, const char* post,
                        size_t post_size, curl_cb_func cb, void* args,
                        const char* cookie);
void curlc_finish(curlc_t*);

void curlc_on_res(curlc_t*);
void curlc_append_res(curlc_t*, char* data);

CURL* curlc_handle(curlc_t*);
void curlc_set_handle(curlc_t*, CURL* c);

const char* curlc_res(curlc_t*);
const char* curlc_req(curlc_t*);

void* curlc_cb_args(curlc_t*);

CURLcode curlc_ret(curlc_t*);
void curlc_set_ret(curlc_t*, CURLcode);
const char* curlc_ret_str(curlc_t*);

size_t curlc_write_cb(char* buffer, size_t sz, size_t nitems, void* userp);

#ifdef __cpluplus
}
#endif

#endif
