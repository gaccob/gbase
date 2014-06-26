#include "net/connector.h"

#define CONNECTOR_BUFFER_SIZE (64 * 1024)

typedef struct connector_t {
    struct handler_t h;
    struct reactor_t* r;

    // malloc by connector
    int8_t flag_rbuf: 1;
    int8_t flag_wbuf : 1;
    connbuffer_t* rbuf;
    connbuffer_t* wbuf;

    // callback function
    connector_read_func on_read;
    void* read_arg;
    connector_close_func on_close;
    void* close_arg;
} connector_t;

static int32_t
_connector_read(struct handler_t* h) {
    char* buffer;
    int32_t nread, nwrite, res;
    connector_t* con = (connector_t*)h;
    nwrite = connbuffer_write_len(con->rbuf);
    assert(nwrite >= 0);
    // read buffer full fail
    if (0 == nwrite) {
        printf("fd[%d] read buffer full.\n", con->h.fd);
        return -1;
    }
    // read socket
    buffer = connbuffer_write_buffer(con->rbuf);
    res = sock_read(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't read now
        if (ERR_EAGAIN == ERRNO || ERR_EWOULDBLOCK == ERRNO || ERR_EINTR == ERRNO) {
            return 0;
        } else {
            return -ERRNO;
        }
    } else if (0 == res) {
        return -1;
    } else {
        connbuffer_write_nocopy(con->rbuf, res);
        buffer = connbuffer_read_buffer(con->rbuf);
        nread = connbuffer_read_len(con->rbuf);
        assert(buffer && nread);
        res = con->on_read(con->h.fd, con->read_arg, buffer, nread);
        return res > 0 ?  connbuffer_read_nocopy(con->rbuf, res) : res;
    }
    return 0;
}

static int32_t
_connector_write(struct handler_t* h) {
    char* buffer;
    int32_t nwrite, res;
    connector_t* con = (connector_t*)h;
    nwrite = connbuffer_read_len(con->wbuf);
    if (nwrite <= 0)
        return 0;
    buffer = connbuffer_read_buffer(con->wbuf);
    res = sock_write(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't write now
        if (ERR_EAGAIN == ERRNO || ERR_EWOULDBLOCK == ERRNO || ERR_EINTR == ERRNO) {
            return 0;
        } else {
            return -ERRNO;
        }
    } else if (0 == res) {
        return -1;
    } else {
        connbuffer_read_nocopy(con->wbuf, res);
        if (res == nwrite) {
            reactor_modify(con->r, &con->h, EVENT_IN);
        }
    }
    return 0;
}

static int32_t
_connector_close(struct handler_t* h) {
    connector_t* con = (connector_t*)h;
    if (con->on_close) {
        con->on_close(con->h.fd, con->close_arg);
    }
    return 0;
}

connector_t*
connector_create(struct reactor_t* r) {
    connector_t* con;
    if (!r)
        return NULL;
    con = (connector_t*)MALLOC(sizeof(connector_t));
    if (!con)
        return NULL;
    memset(con, 0, sizeof(connector_t));
    con->h.fd = INVALID_SOCK;
    con->h.in_func = _connector_read;
    con->h.out_func = _connector_write;
    con->h.close_func = _connector_close;
    con->r = r;
    return con;
}

inline void
connector_set_read_func(connector_t* con,
                        connector_read_func on_read,
                        void* arg) {
    if (con) {
        con->on_read = on_read;
        con->read_arg = arg;
    }
}

inline void
connector_set_close_func(connector_t* con,
                         connector_close_func on_close,
                         void* arg) {
    if (con) {
        con->on_close = on_close;
        con->close_arg = arg;
    }
}

int32_t
connector_release(connector_t* con) {
    if (con) {
        connector_stop(con);
        if (con->flag_rbuf) {
            connbuffer_release(con->rbuf);
        }
        if (con->flag_wbuf) {
            connbuffer_release(con->wbuf);
        }
        FREE(con);
    }
    return 0;
}

inline sock_t
connector_fd(connector_t* con) {
    return con ? con->h.fd : INVALID_SOCK;
}

inline void
connector_set_fd(connector_t* con, sock_t fd) {
    if (con)
        con->h.fd = fd;
}

int32_t
connector_start(connector_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;
    if (!con->rbuf) {
        con->rbuf = connbuffer_create(CONNECTOR_BUFFER_SIZE, MALLOC, FREE);
        assert(con->rbuf);
        con->flag_rbuf = 1;
    }
    if (!con->wbuf) {
        con->wbuf = connbuffer_create(CONNECTOR_BUFFER_SIZE, MALLOC, FREE);
        assert(con->wbuf);
        con->flag_wbuf = 1;
    }
    sock_set_nonblock(con->h.fd);
    sock_set_nodelay(con->h.fd);
    return reactor_register(con->r, &con->h, EVENT_IN);
}

//  return = 0 success
//  return < 0, fail, maybe full
int32_t
connector_send(connector_t* con, const char* buffer, int32_t buflen) {
    if (!con || con->h.fd == INVALID_SOCK || !con->wbuf)
        return -1;
    if (!buffer || buflen < 0)
        return -1;
    int nwrite = connbuffer_write_len(con->wbuf);
    if (buflen > nwrite)
        return -1;
    connbuffer_write(con->wbuf, buffer, buflen);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

int32_t
connector_stop(connector_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;
    reactor_unregister(con->r, &con->h);
    sock_close(con->h.fd);
    con->h.fd = INVALID_SOCK;
    return 0;
}

int32_t
connector_set_rbuf(connector_t* con, connbuffer_t* buf) {
    if (con && !con->rbuf && buf) {
        con->rbuf = buf;
        return 0;
    }
    return -1;
}

int32_t
connector_set_wbuf(connector_t* con, connbuffer_t* buf) {
    if (con && !con->wbuf && buf) {
        con->wbuf = buf;
        return 0;
    }
    return -1;
}
