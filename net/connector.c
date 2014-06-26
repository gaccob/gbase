#include "net/connector.h"

typedef struct connector_t {
    struct handler_t h;
    struct reactor_t* r;
    struct connbuffer_t* read_buf;
    struct connbuffer_t* write_buf;
    connector_read_func read_cb;
    connector_close_func close_cb;
} connector_t;

static int32_t
_connector_read(struct handler_t* h) {
    char* buffer;
    int32_t nread, nwrite, res;
    connector_t* con = (connector_t*)h;
    nwrite = connbuffer_write_len(con->read_buf);
    assert(nwrite >= 0);
    // read buffer full fail
    if (0 == nwrite) {
        printf("fd[%d] read buffer full.\n", con->h.fd);
        return -1;
    }
    // read socket
    buffer = connbuffer_write_buffer(con->read_buf);
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
        connbuffer_write_nocopy(con->read_buf, res);
        buffer = connbuffer_read_buffer(con->read_buf);
        nread = connbuffer_read_len(con->read_buf);
        assert(buffer && nread);
        res = con->read_cb(con->h.fd, buffer, nread);
        if (res > 0) {
            connbuffer_read_nocopy(con->read_buf, res);
        } else {
            return res;
        }
    }
    return 0;
}

static int32_t
_connector_write(struct handler_t* h) {
    char* buffer;
    int32_t nwrite, res;
    connector_t* con = (connector_t*)h;
    nwrite = connbuffer_read_len(con->write_buf);
    if (nwrite <= 0) return 0;
    buffer = connbuffer_read_buffer(con->write_buf);
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
        connbuffer_read_nocopy(con->write_buf, res);
        if (res == nwrite) {
            reactor_modify(con->r, &con->h, EVENT_IN);
        }
    }
    return 0;
}

static int32_t
_connector_close(struct handler_t* h) {
    connector_t* con = (connector_t*)h;
    if (con->close_cb) {
        con->close_cb(con->h.fd);
    }
    return 0;
}

connector_t*
connector_create(struct reactor_t* r,
                 connector_read_func read_cb,
                 connector_close_func close_cb,
                 struct connbuffer_t* read_buf,
                 struct connbuffer_t* write_buf) {
    connector_t* con;
    if (!r || !read_buf || !write_buf) return NULL;
    con = (connector_t*)MALLOC(sizeof(connector_t));
    if (!con) return NULL;
    con->h.fd = -1;
    con->h.in_func = _connector_read;
    con->h.out_func = _connector_write;
    con->h.close_func = _connector_close;
    con->r = r;
    con->read_buf = read_buf;
    con->write_buf = write_buf;
    con->read_cb = read_cb;
    con->close_cb = close_cb;
    return con;
}

int32_t
connector_release(connector_t* con) {
    if (con) {
        connector_stop(con);
        FREE(con);
    }
    return 0;
}

inline int32_t
connector_fd(connector_t* con) {
    return con ? con->h.fd : -1;
}

inline void connector_set_fd(connector_t* con, sock_t fd) {
    if (con) con->h.fd = fd;
}

int32_t
connector_start(connector_t* con) {
    if (!con || con->h.fd < 0) return -1;
    sock_set_nonblock(con->h.fd);
    sock_set_nodelay(con->h.fd);
    return reactor_register(con->r, &con->h, EVENT_IN);
}

//  return = 0 success
//  return < 0, fail, maybe full
int32_t
connector_send(connector_t* con, const char* buffer, int32_t buflen) {
    int32_t nwrite;
    if (!con || !buffer || buflen < 0) return -1;
    nwrite = connbuffer_write_len(con->write_buf);
    if (buflen > nwrite) return -1;
    connbuffer_write(con->write_buf, buffer, buflen);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

int32_t
connector_stop(connector_t* con) {
    if (!con || con->h.fd < 0) return -1;
    reactor_unregister(con->r, &con->h);
    sock_close(con->h.fd);
    con->h.fd = -1;
    return 0;
}

