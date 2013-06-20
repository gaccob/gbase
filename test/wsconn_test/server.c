#include "ds/idtable.h"
#include "os/os_def.h"
#include "net/sock.h"
#include "net/reactor.h"
#include "net/acceptor.h"
#include "net/wsconn.h"

const char* server_addr = "10.0.253.112";
int server_port = 8000;

#define TEST_BUFF_SIZE (1024 * 1024)

int stop_flag;
struct reactor_t* r;
struct idtable_t* con_table;

typedef struct WSCtx
{
	struct wsconn_t* con;
	struct connbuffer_t* read_buf;
    struct connbuffer_t* real_read_buf;
	struct connbuffer_t* write_buf;
}WSCtx;

const char* const stop_cmd = "stop\n";
const char word_cmd = '\n';

int wscon_read(int fd, const char* buffer, int buflen)
{
	struct WSCtx* ctx = idtable_get(con_table, fd);
	if(!ctx)
		return -1;

	printf("fd[%d] read %d bytes\n", fd, buflen);

    if (wsconn_send(ctx->con, buffer, buflen) < 0)
    {
        printf("send back fail\n");
        return -1;
    }
/*
    int32_t i = 0;
    while (i < buflen)
        printf("%c", buffer[i++]);
    printf("\n");
*/
    return buflen;
}

void wscon_build(int fd)
{
    printf("fd[%d] complete handshake\n", fd);
}

void wscon_close(int fd)
{
	printf("fd[%d] close \n", fd);
	struct WSCtx* ctx = idtable_get(con_table, fd);
	assert(ctx);

	connbuffer_release(ctx->read_buf);
	connbuffer_release(ctx->write_buf);
    connbuffer_release(ctx->real_read_buf);
	wsconn_release(ctx->con);
	FREE(ctx);
	idtable_remove(con_table, fd);
}

int accept_read(int fd)
{
	int res;
	struct WSCtx* ctx = (struct WSCtx*)MALLOC(sizeof(struct WSCtx));
	assert(ctx);

	ctx->read_buf = connbuffer_init(TEST_BUFF_SIZE, MALLOC, FREE);
    ctx->real_read_buf = connbuffer_init(TEST_BUFF_SIZE, MALLOC, FREE);
	ctx->write_buf = connbuffer_init(TEST_BUFF_SIZE, MALLOC, FREE);
	assert(ctx->read_buf && ctx->write_buf && ctx->real_read_buf);

	ctx->con = wsconn_init(r, wscon_build, wscon_read, wscon_close, ctx->read_buf,
                           ctx->real_read_buf, ctx->write_buf, fd);
	assert(ctx->con);

	res = idtable_add(con_table, fd, ctx);
	assert(0 == res);

	res = wsconn_start(ctx->con);
	assert(0 == res);

    printf("fd[%d] connected\n", fd);
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
		struct WSCtx* ctx = idtable_get(con_table, i);
		if(ctx)
		{
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

