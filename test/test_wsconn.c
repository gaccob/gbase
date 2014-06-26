#include "base/idtable.h"
#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "net/acceptor.h"
#include "net/wsconn.h"

#include "test.h"

#define TEST_BUFF_SIZE (1024 * 1024)

struct reactor_t* r;
struct idtable_t* con_table;

typedef struct WSCtx {
    struct wsconn_t* con;
} WSCtx;

static int
_wscon_read(int fd, void* arg, const char* buffer, int buflen) {
    int i = 0;
    WSCtx* ctx = idtable_get(con_table, fd);
    if(!ctx) return -1;
    printf("fd[%d] read %d bytes\n", fd, buflen);
    if (wsconn_send(ctx->con, buffer, buflen) < 0) {
        printf("send back fail\n");
        return -1;
    }
    while (i < buflen) {
        printf("%c", buffer[i++]);
    }
    printf("\n");
    return buflen;
}

static void
_wscon_build(int fd, void* arg) {
    printf("fd[%d] complete handshake\n", fd);
}

static void
_wscon_close(int fd, void* arg) {
    WSCtx* ctx = idtable_get(con_table, fd);
    assert(ctx);
    printf("fd[%d] close \n", fd);
    wsconn_release(ctx->con);
    FREE(ctx);
    idtable_remove(con_table, fd);
}

static int
_accept_read(sock_t fd, void* arg) {
    int res;
    struct WSCtx* ctx = (struct WSCtx*)MALLOC(sizeof(struct WSCtx));
    assert(ctx);
    ctx->con = wsconn_create(r);
    assert(ctx->con);
    wsconn_set_build_func(ctx->con, _wscon_build, NULL);
    wsconn_set_read_func(ctx->con, _wscon_read, NULL);
    wsconn_set_close_func(ctx->con, _wscon_close, NULL);
    wsconn_set_fd(ctx->con, fd);
    res = idtable_add(con_table, fd, ctx);
    assert(0 == res);
    res = wsconn_start(ctx->con);
    assert(0 == res);
    printf("fd[%d] connected\n", fd);
    return 0;
}

int
test_ws_server() {
    int res, i;
    struct acceptor_t* acc;
    struct sockaddr_in addr;
    struct WSCtx* ctx;
    con_table = idtable_create(1024);
    assert(con_table);

    r = reactor_create();
    assert(r);

    acc = acceptor_create(r);
    assert(acc);
    acceptor_set_read_func(acc, _accept_read, NULL);

    res = sock_addr_aton(WS_IP, WS_PORT, &addr);
    assert(res == 0);

    res = acceptor_start(acc, (struct sockaddr*)&addr);
    assert(res == 0);

    while (1) {
        res = reactor_dispatch(r, 1);
        if(res < 0) return -1;
        if(res > 0) SLEEP(1);
    }

    acceptor_stop(acc);
    acceptor_release(acc);
    for (i = 0; i < 1024; i++) {
        ctx = idtable_get(con_table, i);
        if (ctx) {
            wsconn_release(ctx->con);
            FREE(ctx);
        }
    }
    idtable_release(con_table);
    reactor_release(r);
    return 0;
}

