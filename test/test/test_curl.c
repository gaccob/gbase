#include <assert.h>
#include "core/os_def.h"
#include "net/curl_pool.h"
#include "net/curl_client.h"

#define TEST_CURL_LOOP 10

static void
_curl_cb(struct curl_client_t* cc, void* args) {
    printf("request: %s\n", curl_client_req(cc));
    printf("response: %s\n", curl_client_res(cc));
}

const char* const url = "https://openmobile.qq.com/user/get_simple_userinfo?access_token=51515EB96D19543E7B1DBA9DFA02F7AA&oauth_consumer_key=100689805&openid=CFFD9B75776BF9EB923E85FF97DE78FE&pf=qzone";

int
test_curl() {
    struct curl_pool_t* cp;
    int32_t ret, i;
    cp = curl_pool_create();
    assert(cp);
    for (i = 0; i < TEST_CURL_LOOP; ++ i) {
        ret = curl_pool_add_get(cp, url, _curl_cb, NULL, NULL);
        assert(0 == ret);
    }
    while (curl_pool_running_count(cp) > 0) {
        curl_pool_run(cp);
        SLEEP(1);
    }
    curl_pool_release(cp);
    return 0;
}

