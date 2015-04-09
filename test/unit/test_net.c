#include <assert.h>
#include <unistd.h>

#include "core/os_def.h"
#include "net/curlp.h"
#include "net/curlc.h"

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
test_net_curl(const char* param) {
    int loop = param ? atoi(param) : 10;
    curlp_t* cp = curlp_create();
    if (!cp) {
        fprintf(stderr, "curl pool create fail\n");
        return -1;
    }

    for (int i = 0; i < loop; ++ i) {
        int ret = curlp_add_get(cp, url, _curl_cb, NULL, NULL);
        if (ret != 0) {
            fprintf(stderr, "curl pool add client fail\n");
            curlp_release(cp);
            return -1;
        }
    }

    while (curlp_running_count(cp) > 0) {
        curlp_poll(cp);
        usleep(100);
    }

    curlp_release(cp);
    return 0;
}

