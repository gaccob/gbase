#ifndef CURL_POOL_H_
#define CURL_POOL_H_

#ifdef __cpluplus
extern "C" {
#endif

#include <stdint.h>
#include <curl/curl.h>
#include "net/curl_client.h"

struct slist_t;
struct hash_t;
struct curl_pool_t;
#define CURL_POOL_DEFAULT_SIZE 50

struct curl_pool_t* curl_pool_init();
void curl_pool_release(struct curl_pool_t*);
int32_t curl_pool_add_get_req(struct curl_pool_t* cp, const char* req,
                              CURL_CALLBACK cb, void* args,
                              const char* cookie);
int32_t curl_pool_add_post_req(struct curl_pool_t* cp, const char* req,
                               const char* post, size_t post_len,
                               CURL_CALLBACK cb, void* args,
                               const char* cookie);
void curl_pool_run(struct curl_pool_t* cp);
int32_t curl_pool_running_count(struct curl_pool_t* cp);

#ifdef __cpluplus
}
#endif

#endif
