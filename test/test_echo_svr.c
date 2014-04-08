#include "core/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "net/acceptor.h"
#include "net/connector.h"
#include "base/idtable.h"

#include "test.h"

int stop_flag;
struct reactor_t* r;
struct idtable_t* con_table;

typedef struct ConCtx {
    struct connector_t* con;
    struct connbuffer_t* read_buf;
    struct connbuffer_t* write_buf;
}ConCtx;

static int
_conn_read(int fd, const char* buffer, int buflen) {
    int res;
    struct ConCtx* ctx = idtable_get(con_table, fd);
    if (!ctx) {
        return -1;
    }

    printf("fd[%d] read %d bytes\n", fd, buflen);
    if (buffer[buflen - 1] != ECHO_CMD_WORD) {
        return 0;
    }
    if (buflen == strlen(ECHO_CMD_STOP)
        && 0 == strncmp(buffer, ECHO_CMD_STOP, buflen)) {
        stop_flag = 1;
        return buflen;
    }

    res = connector_send(ctx->con, buffer, buflen);
    if (res < 0) {
        printf("fd[%d] write %d bytes fail.\n", fd, buflen);
        return -1;
    }
    printf("fd[%d] write %d bytes\n", fd, buflen);
    return buflen;
}

static void
_conn_close(int fd) {
    struct ConCtx* ctx = idtable_get(con_table, fd);
    assert(ctx);
    printf("fd[%d] close \n", fd);
    connbuffer_release(ctx->read_buf);
    connbuffer_release(ctx->write_buf);
    connector_release(ctx->con);
    FREE(ctx);
    idtable_remove(con_table, fd);
}

static int
_accept_read(int fd) {
    int res;
    struct ConCtx* ctx = (struct ConCtx*)MALLOC(sizeof(struct ConCtx));
    assert(ctx);
    ctx->read_buf = connbuffer_create(4096, MALLOC, FREE);
    ctx->write_buf = connbuffer_create(4096, MALLOC, FREE);
    assert(ctx->read_buf && ctx->write_buf);
    ctx->con = connector_create(r, _conn_read, _conn_close, ctx->read_buf, ctx->write_buf);
    assert(ctx->con);
    connector_set_fd(ctx->con, fd);
    res = idtable_add(con_table, fd, ctx);
    assert(0 == res);
    res = connector_start(ctx->con);
    assert(0 == res);
    printf("get connection fd=%d\n", fd);
    return 0;
}

int
test_echo_svr()
{
    int res, i;
    struct acceptor_t* acc;
    struct sockaddr_in addr;
    con_table = idtable_create(1024);
    assert(con_table);

    stop_flag = 0;
    r = reactor_create();
    assert(r);

    acc = acceptor_create(r, _accept_read, NULL);
    assert(acc);

    res = sock_addr_aton(ECHO_IP, ECHO_PORT, &addr);
    assert(0 == res);

    res = acceptor_start(acc, (struct sockaddr*)&addr);
    assert(0 == res);

    while(!stop_flag) {
        res = reactor_dispatch(r, 1);
        if(res < 0) {
            return -1;
        } else if(res > 0) {
            SLEEP(1);
        }
    }

    acceptor_stop(acc);
    acceptor_release(acc);
    for (i = 0; i < 1024; i++) {
        struct ConCtx* ctx = idtable_get(con_table, i);
        if (ctx) {
            connbuffer_release(ctx->read_buf);
            connbuffer_release(ctx->write_buf);
            connector_release(ctx->con);
            FREE(ctx);
        }
    }
    idtable_release(con_table);
    reactor_release(r);
    return 0;
}

