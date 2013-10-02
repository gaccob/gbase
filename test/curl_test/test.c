#include <assert.h>
#include "core/os_def.h"
#include "net/curl_pool.h"
#include "net/curl_client.h"

void curl_cb(struct curl_client_t* cc, void* args)
{
    printf("request: %s\n", curl_client_req(cc)); 
    printf("response: %s\n", curl_client_res(cc));
}

const char* const cgi_url = "119.147.19.43/v3/user/get_info?openid=00000000000000000000000013C10986&openkey=9E47314396BA363B566E245F26663047&pf=qzone&appid=420&format=json&userip=10.0.0.1&sig=IPXBzZreF27V60e6EMQ99mHp6I0%3D";

const char* const cgi_url2 = "https://openmobile.qq.com/user/get_simple_userinfo?access_token=51515EB96D19543E7B1DBA9DFA02F7AA&oauth_consumer_key=100689805&openid=CFFD9B75776BF9EB923E85FF97DE78FE&pf=qzone";

const char* const url = "www.baidu.com";

int main(int argc, char** argv)
{
    struct curl_pool_t* cp;
    int32_t ret;
    int32_t i;
    int32_t loop = (argc != 2 ? 10 : atoi(argv[1]));

    cp = curl_pool_init();
    assert(cp);

    for (i = 0; i < loop; ++ i) {
        ret = curl_pool_add_get_req(cp, cgi_url2, curl_cb, NULL, NULL);
        assert(0 == ret);
    }
    while (curl_pool_running_count(cp) > 0) {
        curl_pool_run(cp);
        SLEEP(1);     
    }

    curl_pool_release(cp);
    return 0;
}

