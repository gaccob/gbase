#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <pthread.h>

#include "base/idtable.h"
#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "net/acceptor.h"
#include "net/connector.h"

#define ECHO_IP "0.0.0.0"
#define ECHO_PORT 8000

static int _insts;
static reactor_t* _s_reactor;
static idtable_t* _s_cons;

typedef struct ConCtx {
    con_t* con;
} ConCtx;

static int
_conn_read(sock_t fd, void* arg, const char* buffer, int buflen) {

    ConCtx* ctx = idtable_get(_s_cons, fd);
    if (!ctx) {
        fprintf(stderr, "server connection[fd=%d] find fail\n", fd);
        return -1;
    }

    printf("server fd[%d] read %d bytes\n", fd, buflen);

    int res = con_send(ctx->con, buffer, buflen);
    if (res < 0) {
        fprintf(stderr, "server fd[%d] write %d bytes fail.\n", fd, buflen);
        return -1;
    }

    printf("server fd[%d] write %d bytes\n", fd, buflen);
    return buflen;
}

static void
_conn_close(sock_t fd, void* arg) {
    ConCtx* ctx = idtable_get(_s_cons, fd);
    assert(ctx);

    printf("server fd[%d] close \n", fd);
    -- _insts;

    con_release(ctx->con);
    FREE(ctx);
    idtable_remove(_s_cons, fd);
}

static int
_accept_read(sock_t fd, void* arg) {
    ConCtx* ctx = (ConCtx*)MALLOC(sizeof(ConCtx));
    assert(ctx);

    ctx->con = con_create(_s_reactor);
    assert(ctx->con);

    con_set_read_func(ctx->con, _conn_read, NULL);
    con_set_close_func(ctx->con, _conn_close, NULL);
    con_set_sock(ctx->con, fd);

    int res = idtable_add(_s_cons, fd, ctx);
    assert(0 == res);

    res = con_start(ctx->con);
    assert(0 == res);

    printf("server get connection fd=%d\n", fd);
    return 0;
}

/////////////////////////////////////////////////////////////////////////////

static void*
echo_server(void* arg) {

    _s_cons = idtable_create(1024);
    _s_reactor = reactor_create();
    assert(_s_cons && _s_reactor);

    printf("echo server start\n");

    acc_t* acc = acc_create(_s_reactor);
    assert(acc);
    acc_set_read_func(acc, _accept_read, NULL);

    struct sockaddr_in addr;
    int res = sock_addr_aton(ECHO_IP, ECHO_PORT, &addr);
    assert(0 == res);

    res = acc_start(acc, (struct sockaddr*)&addr);
    assert(0 == res);

    while(_insts > 0) {
        res = reactor_dispatch(_s_reactor, 1);
        if(res < 0) {
            fprintf(stderr, "server dispatch fail: %d\n", res);
            return NULL;
        } else if(res > 0) {
            usleep(100);
        }
    }

    printf("echo server stop\n");

    acc_stop(acc);
    acc_release(acc);
    for (int i = 0; i < 1024; i++) {
        ConCtx* ctx = idtable_get(_s_cons, i);
        if (ctx) {
            con_release(ctx->con);
            FREE(ctx);
        }
    }

    idtable_release(_s_cons);
    reactor_release(_s_reactor);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

static void*
echo_client(void* arg) {

    uint64_t tid = (uint64_t)pthread_self();

    sock_t sock = sock_tcp();
    assert(sock >= 0);

    struct timeval tv;
    tv.tv_sec = 10;
    tv.tv_usec = 0;
    int res = sock_nonblock_connect(sock, ECHO_IP, ECHO_PORT, tv);
    assert(0 == res);

    printf("thread[%llx] client connect fd=%d\n", tid, sock);

    int loop = arg ? atoi((char*)arg) : 3;
    while (loop -- > 0) {

        char buffer[1024];
        snprintf(buffer, sizeof(buffer), "thread[%llx] say hello loop=%d", tid, loop);
        printf("%s\n", buffer);

        int nsend = 0;
        int buflen = strlen(buffer);
        while (nsend < buflen) {
            int res = write(sock, buffer, buflen);
            if(res > 0) {
                nsend += res;
            } else {
                usleep(1);
            }
        }

        char recv[1024];
        memset(recv, 0, sizeof(recv));

        int nread = 0;
        while (nread < buflen) {
            res = read(sock, recv + nread, sizeof(recv) - nread);
            if (res > 0) {
                nread += res;
            } else if (res == 0) {
                printf("\necho server quit.\n");
                return 0;
            } else {
                usleep(1);
            }
        }
        printf("echo: %s\n", recv);

        if (strcmp(recv, buffer)) {
            fprintf(stderr, "echo != sendings");
            sock_close(sock);
            return NULL;
        }
    }

    sock_close(sock);
    return NULL;
}

/////////////////////////////////////////////////////////////////////////////

int
test_net_echo(const char* param) {
    _insts = param ? atoi((char*)param) : 3;

    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, (1 << 20));

    pthread_t t_server;
    pthread_create(&t_server, &attr, echo_server, NULL);
    usleep(1000);

    pthread_t t_client[_insts];
    for (int i = 0; i < _insts; ++ i) {
        pthread_create(&t_client[i], &attr, echo_client, NULL);
    }

    for (int i = 0; i < _insts; ++ i) {
        pthread_join(t_client[i], NULL);
    }
    pthread_join(t_server, NULL);

    return 0;
}

