#include "tcp_test.h"
#include "core/idtable.h"

int stop_flag;
struct reactor_t* r;
struct idtable_t* con_table;

typedef struct ConCtx
{
    struct connector_t* con;
    struct connbuffer_t* read_buf;
    struct connbuffer_t* write_buf;
}ConCtx;

int conn_read(int fd, const char* buffer, int buflen)
{
    int res;
    struct ConCtx* ctx = idtable_get(con_table, fd);
    if(!ctx)
        return -1;

    printf("fd[%d] read %d bytes\n", fd, buflen);
    if(buffer[buflen - 1] != word_cmd)
        return 0;

    if(buflen == strlen(stop_cmd) && 0 == strncmp(buffer, stop_cmd, buflen))
    {
        stop_flag = 1;
        return buflen;
    }

    res = connector_send(ctx->con, buffer, buflen);
    if (res < 0)
    {
        printf("fd[%d] write %d bytes fail.\n", fd, buflen);
        return -1;
    }
    printf("fd[%d] write %d bytes\n", fd, buflen);
    return buflen;
}

void conn_close(int fd)
{
    struct ConCtx* ctx = idtable_get(con_table, fd);
    assert(ctx);
    printf("fd[%d] close \n", fd);

    connbuffer_release(ctx->read_buf);
    connbuffer_release(ctx->write_buf);
    connector_release(ctx->con);
    FREE(ctx);
    idtable_remove(con_table, fd);
}

int accept_read(int fd)
{
    int res;
    struct ConCtx* ctx = (struct ConCtx*)MALLOC(sizeof(struct ConCtx));
    assert(ctx);

    ctx->read_buf = connbuffer_init(4096, MALLOC, FREE);
    ctx->write_buf = connbuffer_init(4096, MALLOC, FREE);
    assert(ctx->read_buf && ctx->write_buf);

    ctx->con = connector_init(r, conn_read, conn_close, ctx->read_buf, ctx->write_buf, fd);
    assert(ctx->con);

    res = idtable_add(con_table, fd, ctx);
    assert(0 == res);

    res = connector_start(ctx->con);
    assert(0 == res);

    printf("get connection fd=%d\n", fd);

    return 0;
}

int main()
{
    int res, i;
    struct acceptor_t* acc;
    struct sockaddr_in addr;

    con_table = idtable_init(1024);
    if(!con_table)
        return 0;

    stop_flag = 0;
    r = reactor_init();
    if(!r)
        return -1;

    acc = acceptor_init(r, accept_read, NULL);
    if(!acc)
        return -1;

    res = sock_addr_aton(server_addr, server_port, &addr);
    if(res < 0)
        return -1;

    res = acceptor_start(acc, (struct sockaddr*)&addr);
    if(res < 0)
        return -1;

    while(!stop_flag)
    {
        res = reactor_dispatch(r, 1);
        if(res < 0)
            return -1;
        if(res > 0)
            SLEEP(1);
    }

    acceptor_stop(acc);
    acceptor_release(acc);

    for(i = 0; i < 1024; i++)
    {
        struct ConCtx* ctx = idtable_get(con_table, i);
        if(ctx)
        {
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

