#include <assert.h>
#include <unistd.h>
#include "core/os_def.h"
#include "net/curlp.h"
#include "net/curlc.h"

#define TEST_CURL_LOOP 10

static void
_curl_cb(curlc_t* cc, void* args) {
    printf("request: %s\n", curlc_req(cc));
    printf("response: %s\n", curlc_res(cc));
}

const char* const url = "https://openmobile.qq.com/user/get_simple_userinfo"
                        "?access_token=51515EB96D19543E7B1DBA9DFA02F7AA"
                        "&oauth_consumer_key=100689805"
                        "&openid=CFFD9B75776BF9EB923E85FF97DE78FE"
                        "&pf=qzone";

int
test_curl() {
    curlp_t* cp = curlp_create();
    assert(cp);
    for (int i = 0; i < TEST_CURL_LOOP; ++ i) {
        int ret = curlp_add_get(cp, url, _curl_cb, NULL, NULL);
        assert(0 == ret);
    }
    while (curlp_running_count(cp) > 0) {
        curlp_poll(cp);
        usleep(100);
    }
    curlp_release(cp);
    return 0;
}

