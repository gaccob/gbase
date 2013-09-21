#include <assert.h>
#include "core/os_def.h"
#include "net/curl_pool.h"
#include "net/curl_client.h"

void curl_cb(struct curl_client_t* cc, void* args)
{
    printf("request: %s\n", curl_client_req(cc)); 
    printf("response: %s\n", curl_client_res(cc));
}

const char* const cgi_url = "http://119.147.19.43/v3/user/get_info?openid=00000000000000000000000013C10986&openkey=9E47314396BA363B566E245F26663047&pf=qzone&appid=420&format=json&userip=10.0.0.1&sig=IPXBzZreF27V60e6EMQ99mHp6I0%3D";

int main()
{
    struct curl_pool_t* cp;
    int32_t ret;

    cp = curl_pool_init();
    assert(cp);

    ret = curl_pool_add_get_req(cp, cgi_url, curl_cb, NULL, NULL);
    assert(0 == ret);

    while (curl_pool_running_count(cp) > 0) {
        curl_pool_run(cp);
        SLEEP(1);     
    }

    curl_pool_release(cp);
    return 0;
}

