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
    struct connbuffer_t* read_buf;
    struct connbuffer_t* real_read_buf;
    struct connbuffer_t* write_buf;
} WSCtx;

static int
_wscon_read(int fd, const char* buffer, int buflen) {
    int i = 0;
    struct WSCtx* ctx = idtable_get(con_table, fd);
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
_wscon_build(int fd) {
    printf("fd[%d] complete handshake\n", fd);
}

static void
_wscon_close(int fd) {
    struct WSCtx* ctx = idtable_get(con_table, fd);
    assert(ctx);
    printf("fd[%d] close \n", fd);
    connbuffer_release(ctx->read_buf);
    connbuffer_release(ctx->write_buf);
    connbuffer_release(ctx->real_read_buf);
    wsconn_release(ctx->con);
    FREE(ctx);
    idtable_remove(con_table, fd);
}

static int
_accept_read(int fd) {
    int res;
    struct WSCtx* ctx = (struct WSCtx*)MALLOC(sizeof(struct WSCtx));
    assert(ctx);
    ctx->read_buf = connbuffer_create(TEST_BUFF_SIZE, MALLOC, FREE);
    ctx->real_read_buf = connbuffer_create(TEST_BUFF_SIZE, MALLOC, FREE);
    ctx->write_buf = connbuffer_create(TEST_BUFF_SIZE, MALLOC, FREE);
    assert(ctx->read_buf && ctx->write_buf && ctx->real_read_buf);
    ctx->con = wsconn_create(r, _wscon_build, _wscon_read, _wscon_close, ctx->read_buf,
                             ctx->real_read_buf, ctx->write_buf);
    assert(ctx->con);
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
    acc = acceptor_create(r, _accept_read, NULL);
    assert(acc);
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
            connbuffer_release(ctx->read_buf);
            connbuffer_release(ctx->write_buf);
            connbuffer_release(ctx->real_read_buf);
            wsconn_release(ctx->con);
            FREE(ctx);
        }
    }
    idtable_release(con_table);
    reactor_release(r);
    return 0;
}

