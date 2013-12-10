#include <stdio.h>
#include <stdint.h>
#include <assert.h>

#include <openssl/dh.h>
#include <openssl/pem.h>

#include "util/util_time.h"

#define DH_PARAMETER_LEN  256
#define DH_PEM_FILE "dh.pem"

void test_dh_perf()
{
    DH* dh;
    uint8_t* key;
    int ret, errcode;

    dh = DH_new();
    assert(dh);

    ret = DH_generate_parameters_ex(dh, DH_PARAMETER_LEN, DH_GENERATOR_2, NULL);
    assert(ret == 1);

    ret = DH_check(dh, &errcode);
    assert(ret);

    ret = DH_generate_key(dh);
    assert(ret == 1);

    ret = DH_check_pub_key(dh, dh->pub_key, &errcode);
    assert(ret == 1);

    key = (uint8_t*)calloc(DH_size(dh), sizeof(uint8_t));
    assert(key);

    ret = DH_compute_key(key, dh->pub_key, dh);
    assert(ret > 0);

    DH_free(dh);
    free(key);
}

void test_dh(int save_pem)
{
    DH* server = NULL;
    DH* client = NULL;
    int i, ret, errcode;
    uint8_t* key;
    FILE* pem;

    pem = fopen(DH_PEM_FILE, "r");
    if (pem) {
        server = PEM_read_DHparams(pem, NULL, NULL, NULL);
    } else {
        server = DH_new();

        // generator dh parameters
        ret = DH_generate_parameters_ex(server, DH_PARAMETER_LEN,
            DH_GENERATOR_2, NULL);
        if (ret != 1) {
            printf("server generate parameters fail: %d\n", ret);
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
    printf("P: %s\n", BN_bn2hex(server->p));
    printf("G: %s\n\n", BN_bn2hex(server->g));

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
    key = (uint8_t*)calloc(DH_size(server), sizeof(uint8_t));
    DH_compute_key(key, server->pub_key, client);
    if (ret <= 0) {
        printf("client compute key fail: %d\n", ret);
        goto DH_KEY_FAIL;
    }
    printf("server public key: %s\n", BN_bn2hex(server->pub_key));
    printf("client calculate key: ");
    for (i = 0; i < DH_size(server); ++ i) {
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
    printf("client public key: %s\n", BN_bn2hex(client->pub_key));
    printf("server calculate key: ");
    for (i = 0; i < DH_size(server); ++ i) {
        printf("%X%X", (key[i] >> 4) & 0xf, key[i] & 0xf);
    }
    printf("\n\n");

    // save pem file if first time
    if (!pem && save_pem == 0) {
        pem = fopen(DH_PEM_FILE, "w");
        if (pem) {
            PEM_write_DHparams(pem, server);
            printf("save dh pem for first time\n");
        }
    }

DH_KEY_FAIL:
    free(key);
DH_FAIL:
    DH_free(server);
    DH_free(client);
    if (pem) {
        fclose(pem);
    }
}

int main(int argc, char** argv)
{
    int i;
    struct timeval tv;
    char timestamp[64];

    if (argc == 2) {
        // peformance test
        util_gettimeofday(&tv, NULL);
        util_timestamp(&tv, timestamp, 64);
        printf("%s\n", timestamp);
        for (i = 0; i < atoi(argv[1]); ++ i) {
            test_dh_perf();
        }
        util_gettimeofday(&tv, NULL);
        util_timestamp(&tv, timestamp, 64);
        printf("%s\n", timestamp);
    } else {
        // logic test
        test_dh(0);
    }

    return 0;
}

