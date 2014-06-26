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
} ConCtx;

static int
_conn_read(sock_t fd, void* arg, const char* buffer, int buflen) {
    int res;
    struct ConCtx* ctx = idtable_get(con_table, fd);
    if (!ctx)
        return -1;

    printf("fd[%d] read %d bytes\n", fd, buflen);
    if (buffer[buflen - 1] != ECHO_CMD_WORD)
        return 0;

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
_conn_close(sock_t fd, void* arg) {
    struct ConCtx* ctx = idtable_get(con_table, fd);
    assert(ctx);
    printf("fd[%d] close \n", fd);
    connector_release(ctx->con);
    FREE(ctx);
    idtable_remove(con_table, fd);
}

static int
_accept_read(sock_t fd, void* arg) {
    int res;
    struct ConCtx* ctx = (struct ConCtx*)MALLOC(sizeof(struct ConCtx));
    assert(ctx);
    ctx->con = connector_create(r);
    assert(ctx->con);
    connector_set_read_func(ctx->con, _conn_read, NULL);
    connector_set_close_func(ctx->con, _conn_close, NULL);
    connector_set_fd(ctx->con, fd);
    res = idtable_add(con_table, fd, ctx);
    assert(0 == res);
    res = connector_start(ctx->con);
    assert(0 == res);
    printf("get connection fd=%d\n", fd);
    return 0;
}

int
test_echo_svr() {
    int res, i;
    struct acceptor_t* acc;
    struct sockaddr_in addr;
    con_table = idtable_create(1024);
    assert(con_table);

    stop_flag = 0;
    r = reactor_create();
    assert(r);

    acc = acceptor_create(r);
    assert(acc);
    acceptor_set_read_func(acc, _accept_read, NULL);

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
            connector_release(ctx->con);
            FREE(ctx);
        }
    }
    idtable_release(con_table);
    reactor_release(r);
    return 0;
}

