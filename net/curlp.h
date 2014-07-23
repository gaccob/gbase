#ifndef CURL_POOL_H_
#define CURL_POOL_H_

#ifdef __cpluplus
extern "C" {
#endif

#include <stdint.h>
#include <curl/curl.h>
#include "net/curlc.h"

typedef struct curl_pool_t curlp_t;
#define CURL_POOL_DEFAULT_SIZE 50

curlp_t* curlp_create();
void curlp_release(curlp_t*);
int curlp_add_get(curlp_t*, const char* req, curl_cb_func cb,
                  void* args, const char* cookie);
int curlp_add_post(curlp_t*, const char* req, const char* post,
                   size_t post_len, curl_cb_func cb, void* args,
                   const char* cookie);
void curlp_poll(curlp_t*);
int curlp_running_count(curlp_t*);

#ifdef __cpluplus
}
#endif

#endif
