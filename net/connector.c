#include "net/connector.h"

#define CONNECTOR_BUFFER_SIZE (64 * 1024)

struct connector_t {
    handler_t h;
    reactor_t* r;

    // malloc by connector
    int8_t flag_rbuf: 1;
    int8_t flag_wbuf : 1;
    buffer_t* rbuf;
    buffer_t* wbuf;

    // callback function
    con_read_func on_read;
    void* read_arg;
    con_close_func on_close;
    void* close_arg;
};

static int
_con_read(handler_t* h) {
    con_t* con = (con_t*)h;
    int nwrite = buffer_write_len(con->rbuf);
    assert(nwrite >= 0);
    // read buffer full fail
    if (0 == nwrite) {
        printf("fd[%d] read buffer full.\n", con->h.fd);
        return -1;
    }
    // read socket
    char* buffer = buffer_write_buffer(con->rbuf);
    int res = read(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't read now
        if (EAGAIN == errno || EINTR == errno) {
            return 0;
        } else {
            return -errno;
        }
    } else if (0 == res) {
        return -1;
    } else {
        buffer_write_nocopy(con->rbuf, res);
        buffer = buffer_read_buffer(con->rbuf);
        int nread = buffer_read_len(con->rbuf);
        assert(buffer && nread);
        res = con->on_read(con->h.fd, con->read_arg, buffer, nread);
        return res > 0 ?  buffer_read_nocopy(con->rbuf, res) : res;
    }
    return 0;
}

static int
_con_write(handler_t* h) {
    con_t* con = (con_t*)h;
    int nwrite = buffer_read_len(con->wbuf);
    if (nwrite <= 0)
        return 0;

    char* buffer = buffer_read_buffer(con->wbuf);
    int res = write(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't write now
        if (EAGAIN == errno || EINTR == errno) {
            return 0;
        } else {
            return -errno;
        }
    } else if (0 == res) {
        return -1;
    } else {
        buffer_read_nocopy(con->wbuf, res);
        if (res == nwrite) {
            reactor_modify(con->r, &con->h, EVENT_IN);
        }
    }
    return 0;
}

static int
_con_close(struct handler_t* h) {
    con_t* con = (con_t*)h;
    if (con->on_close) {
        con->on_close(con->h.fd, con->close_arg);
    }
    return 0;
}

con_t*
con_create(struct reactor_t* r) {
    if (!r)
        return NULL;
    con_t* con = (con_t*)MALLOC(sizeof(con_t));
    if (!con)
        return NULL;
    memset(con, 0, sizeof(con_t));
    con->h.fd = INVALID_SOCK;
    con->h.in_func = _con_read;
    con->h.out_func = _con_write;
    con->h.close_func = _con_close;
    con->r = r;
    return con;
}

inline void
con_set_read_func(con_t* con, con_read_func on_read, void* arg) {
    if (con) {
        con->on_read = on_read;
        con->read_arg = arg;
    }
}

inline void
con_set_close_func(con_t* con, con_close_func on_close, void* arg) {
    if (con) {
        con->on_close = on_close;
        con->close_arg = arg;
    }
}

int
con_release(con_t* con) {
    if (con) {
        con_stop(con);
        if (con->flag_rbuf) {
            buffer_release(con->rbuf);
        }
        if (con->flag_wbuf) {
            buffer_release(con->wbuf);
        }
        FREE(con);
    }
    return 0;
}

inline sock_t
con_sock(con_t* con) {
    return con ? con->h.fd : INVALID_SOCK;
}

inline void
con_set_sock(con_t* con, sock_t fd) {
    if (con)
        con->h.fd = fd;
}

int
con_start(con_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;
    if (!con->rbuf) {
        con->rbuf = buffer_create(CONNECTOR_BUFFER_SIZE, MALLOC, FREE);
        assert(con->rbuf);
        con->flag_rbuf = 1;
    }
    if (!con->wbuf) {
        con->wbuf = buffer_create(CONNECTOR_BUFFER_SIZE, MALLOC, FREE);
        assert(con->wbuf);
        con->flag_wbuf = 1;
    }
    sock_set_nonblock(con->h.fd);
    sock_set_nodelay(con->h.fd);
    return reactor_register(con->r, &con->h, EVENT_IN);
}

//  return = 0 success
//  return < 0, fail, maybe full
int
con_send(con_t* con, const char* buffer, int buflen) {
    if (!con || con->h.fd == INVALID_SOCK || !con->wbuf)
        return -1;
    if (!buffer || buflen < 0)
        return -1;
    int nwrite = buffer_write_len(con->wbuf);
    if (buflen > nwrite)
        return -1;
    buffer_write(con->wbuf, buffer, buflen);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

int
con_stop(con_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;
    reactor_unregister(con->r, &con->h);
    sock_close(con->h.fd);
    con->h.fd = INVALID_SOCK;
    return 0;
}

int
con_set_rbuf(con_t* con, buffer_t* buf) {
    if (con && !con->rbuf && buf) {
        con->rbuf = buf;
        return 0;
    }
    return -1;
}

int
con_set_wbuf(con_t* con, buffer_t* buf) {
    if (con && !con->wbuf && buf) {
        con->wbuf = buf;
        return 0;
    }
    return -1;
}
