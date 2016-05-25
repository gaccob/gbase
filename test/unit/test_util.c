#include <assert.h>
#include <sys/time.h>
#include <openssl/dh.h>
#include <openssl/pem.h>

#include "util/base64.h"
#include "util/sha1.h"
#include "util/random.h"
#include "util/encode.h"
#include "util/util_time.h"

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_base64(const char* param) {
    const char* const src = "Man is distinguished, not only by his reason,"
        " but by this singular passion from other animals, which is a lust"
        " of the mind, that by a perseverance of delight in the continued"
        " and indefatigable generation of knowledge, exceeds the short"
        " vehemence of any carnal pleasure.";
    char dst[1024], src2[1024];
    memset(dst, 0, sizeof(dst));
    memset(src2, 0, sizeof(src2));
    if (base64_encode(dst, src, strlen(src)) < 0) {
        fprintf(stderr, "base64 encode fail\n");
        return -1;
    }
    if (base64_decode(src2, dst, strlen(dst)) < 0) {
        fprintf(stderr, "base64 decode fail\n");
        return -1;
    }
    if (strcmp(src2, src)) {
        fprintf(stderr, "base64 decode: %s\n", src2);
        return -1;
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static int
_test_util_wscode(const char* req, const char* res) {
    char key[64], sha[128], base64[128];
    memset(sha, 0, sizeof(sha));
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", req);
    sha1(sha, key, strlen(key) * 8);
    if (base64_encode(base64, sha, strlen(sha)) < 0) {
        fprintf(stderr, "base64 encode fail\n");
        return -1;
    }
    printf("wscode [%s]->[%s]\n", req, base64);
    if (res && strcmp(base64, res)) {
        fprintf(stderr, "fail, expect [%s]\n", res);
        return -1;
    }
    return 0;
}

int
test_util_wscode(const char* param) {
    char* req = "2SCVXUeP9cTjV+0mWB8J6A==";
    char* res = "mLDKNeBNWz6T9SxU+o0Fy/HgeSw=";
    int ret = 0;
    ret |= _test_util_wscode(req, res);
    if (param) {
        ret |= _test_util_wscode(param, NULL);
    }
    return ret;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_random(const char* param) {
    int32_t i;
    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < 16; ++ i) {
        rand_gen();
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_shuffle(const char* param) {
    int size = param ? atoi(param) : 52;
	int i, cards[size];
    rand_seed((uint32_t)time(NULL));
    for (i = 0; i < size; ++ i)
        cards[i] = i;
    rand_shuffle(cards, size);
    for (i = 0; i < size; ++ i)
        printf("%d ", cards[i]);
    printf("\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_unicode(const char* param) {
    int unicode = 0;
    char* utf8 = "这是一个测试串";
    char** src = &utf8;
    while (0 == utf8_unicode(src, &unicode)) {
        printf("%d ", unicode);
    }
    printf("\n");
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

int
test_util_dh(const char* param) {

/* TODO:  openssl -> crypto

    DH* server = NULL;
    DH* client = NULL;
    int ret, errcode;
    const char* file = param ? param : "dh.pem";
    FILE* pem = fopen(file, "r");
    if (pem) {
        server = PEM_read_DHparams(pem, NULL, NULL, NULL);
    } else {
        server = DH_new();
        // generator dh parameters
        ret = DH_generate_parameters_ex(server, 256, DH_GENERATOR_2, NULL);
        if (ret != 1) {
            fprintf(stderr, "server generate parameters fail: %d\n", ret);
            goto DH_FAIL;
        }
        // check parameters
        ret = DH_check(server, &errcode);
        if (ret != 1) {
            printf("server paramters check fail: %d\n", errcode);
            goto DH_FAIL;
        }
    }

    // P & G
    char* pp = BN_bn2hex(server->p);
    printf("P: %s\n", pp);
    OPENSSL_free(pp);
    char* pg = BN_bn2hex(server->g);
    printf("G: %s\n\n", pg);
    OPENSSL_free(pg);

    // generator key
    ret = DH_generate_key(server);
    if (ret != 1) {
        printf("server generate key fail: %d\n", ret);
        goto DH_FAIL;
    }

    // check public key
    ret = DH_check_pub_key(server, server->pub_key, &errcode);
    if (ret != 1) {
        printf("server check public key fail: %d\n", errcode);
        goto DH_FAIL;
    }

    // duplicate client for test
    client = DH_new();
    client->p = BN_dup(server->p);
    client->g = BN_dup(server->g);

    // client generate key
    ret = DH_generate_key(client);
    if (ret != 1) {
        printf("client generate key fail: %d\n", ret);
        goto DH_FAIL;
    }

    // check client public key
    ret = DH_check_pub_key(client, client->pub_key, &errcode);
    if (ret != 1) {
        printf("client check public key fail: %d\n", errcode);
        goto DH_FAIL;
    }

    // client compute key: params(server public key, p, g)
    uint8_t* key = (uint8_t*)calloc(DH_size(server), sizeof(uint8_t));
    DH_compute_key(key, server->pub_key, client);
    if (ret <= 0) {
        printf("client compute key fail: %d\n", ret);
        goto DH_KEY_FAIL;
    }

    char* s_pub_key = BN_bn2hex(server->pub_key);
    printf("server public key: %s\n", s_pub_key);
    OPENSSL_free(s_pub_key);

    printf("client calculate key: ");
    for (int i = 0; i < DH_size(server); ++ i) {
        printf("%X%X", (key[i] >> 4) & 0xf, key[i] & 0xf);
    }
    printf("\n\n");

    // server compute key: params(client public key, p, g)
    free(key);
    key = (uint8_t*)calloc(DH_size(client), sizeof(uint8_t));
    DH_compute_key(key, client->pub_key, server);
    if (ret <= 0) {
        printf("server compute key fail: %d\n", ret);
        goto DH_KEY_FAIL;
    }

    char* c_pub_key = BN_bn2hex(server->pub_key);
    printf("client public key: %s\n", c_pub_key);
    OPENSSL_free(c_pub_key);

    printf("server calculate key: ");
    for (int i = 0; i < DH_size(server); ++ i) {
        printf("%X%X", (key[i] >> 4) & 0xf, key[i] & 0xf);
    }
    printf("\n\n");

    // save pem file if first time
    if (!pem) {
        pem = fopen(file, "w");
        if (pem) {
            PEM_write_DHparams(pem, server);
            printf("save dh pem for first time\n\n");
        }
    }

    // success
    free(key);
    DH_free(server);
    DH_free(client);
    if (pem) {
        fclose(pem);
    }
    return 0;

DH_KEY_FAIL:
    free(key);
DH_FAIL:
    DH_free(server);
    DH_free(client);
    if (pem) {
        fclose(pem);
    }
    return -1;
*/
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

static int
_test_dh_perf() {
    int ret, errcode;

    DH* dh = DH_new();
    if (!dh) {
        fprintf(stderr, "DH create fail\n");
        return -1;
    }

    ret = DH_generate_parameters_ex(dh, 256, DH_GENERATOR_2, NULL);
    if (ret != 1) {
        fprintf(stderr, "DH generate param ex fail\n");
        DH_free(dh);
        return -1;
    }

    ret = DH_check(dh, &errcode);
    if (ret < 0) {
        fprintf(stderr, "DH_check fail: %d\n", errcode);
        DH_free(dh);
        return -1;
    }

    ret = DH_generate_key(dh);
    if (ret != 1) {
        fprintf(stderr, "DH_generate_key fail\n");
        DH_free(dh);
        return -1;
    }

    ret = DH_check_pub_key(dh, dh->pub_key, &errcode);
    if (ret != 1) {
        fprintf(stderr, "DH_check_pub_key fail:%d\n", errcode);
        DH_free(dh);
        return -1;
    }

    uint8_t* key = (uint8_t*)calloc(DH_size(dh), sizeof(uint8_t));
    assert(key);

    ret = DH_compute_key(key, dh->pub_key, dh);
    if (ret <= 0) {
        fprintf(stderr, "DH_compute_key fail\n");
        DH_free(dh);
        free(key);
        return -1;
    }

    DH_free(dh);
    free(key);
    return 0;
}

int
test_util_dh_perf(const char* param) {
    struct timeval tv;
    char timestamp[64];
    gettimeofday(&tv, NULL);
    util_timestamp(&tv, timestamp, 64);
    printf("%s start\n", timestamp);

    int loop = param ? atoi(param) : 10;
    for (int i = 0; i < loop; ++ i) {
        _test_dh_perf();
    }

    printf("test %d times\n", loop);

    gettimeofday(&tv, NULL);
    util_timestamp(&tv, timestamp, 64);
    printf("%s end\n", timestamp);
    return 0;
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

