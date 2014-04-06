#ifndef CURL_CLIENT_H_
#define CURL_CLIENT_H_

#ifdef __cpluplus
extern "C" {
#endif

#include <stdint.h>
#include <curl/curl.h>

struct curl_client_t;
typedef void (*CURL_CALLBACK)(struct curl_client_t*, void* args);

#define CURL_MAX_REQUEST_LEN 4096
#define CURL_MAX_RESPONSE_LEN 4096

#define CURL_CONNECT_TIMEOUT_MS 3000
#define CURL_TIMEOUT_MS 3000

struct curl_client_t* curl_client_create();
void curl_client_release(struct curl_client_t*);

// only alloc memory, no curl init
struct curl_client_t* curl_client_raw_create();
void curl_client_raw_release(struct curl_client_t*);

int32_t curl_client_init_get(struct curl_client_t*, const char* req,
                             CURL_CALLBACK cb, void* args,
                             const char* cookie);
int32_t curl_client_init_post(struct curl_client_t*, const char* req,
                              const char* post, size_t post_size,
                              CURL_CALLBACK cb, void* args,
                              const char* cookie);
void curl_client_finish(struct curl_client_t*);

void curl_client_on_res(struct curl_client_t*);
void curl_client_append_res(struct curl_client_t*, char* data);

CURL* curl_client_handle(struct curl_client_t*);
void curl_client_set_handle(struct curl_client_t*, CURL* c);

const char* curl_client_res(struct curl_client_t*);
const char* curl_client_req(struct curl_client_t*);

void* curl_client_cb_args(struct curl_client_t*);

CURLcode curl_client_ret(struct curl_client_t*);
void curl_client_set_ret(struct curl_client_t*, CURLcode);
const char* curl_client_ret_str(struct curl_client_t*);

size_t curl_client_write_cb(char* buffer, size_t sz,
                            size_t nitems, void* userp);

#ifdef __cpluplus
}
#endif

#endif
