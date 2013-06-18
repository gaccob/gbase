#include "net/wsconn.h"
#include "ds/sha1.h"
#include "os/util.h"

typedef struct wsconn_t
{
    struct handler_t h;
    struct reactor_t* r;
    struct connbuffer_t* read_buf;
    struct connbuffer_t* write_buf;
    wsconn_build_func build_cb;
    wsconn_read_func read_cb;
    wsconn_close_func close_cb;
    int8_t status;
}wsconn_t;

enum {
    WS_INIT,
    WS_ESTABLISHED,
    WS_FINI,
};

#define LOWERCASE_ASSIGN(s, a) \
    { s = ((a) >= 'A' && (a) <= 'Z') ? a - ('A' - 'a') : a; }

#define WS_FRAME_MAX_SIZE 1024

enum {
    WS_FLAG_NULL = 0x0,
    WS_FLAG_UPGRAGE = 0x0001,
    WS_FLAG_CONNECTION = 0x0002,
    WS_FLAG_HOST = 0x0004,
    WS_FLAG_ORIGIN = 0x0008,
    WS_FLAG_SEC_ORIGIN = 0x0010,
    WS_FLAG_SEC_KEY = 0x0020,
    WS_FLAG_SEC_VERSION = 0x0040,
    WS_FLAG_SEC_KEY1 = 0x0080,
    WS_FLAG_SEC_KEY2 = 0x0100,
    WS_FLAG_SEC_PROTOCOL = 0x0200,
};

#define WS_FLASH (WS_FLAG_UPGRAGE \
                 | WS_FLAG_CONNECTION \
                 | WS_FLAG_HOST \
                 | WS_FLAG_ORIGIN)
#define WS_SHA1 (WS_FLAG_UPGRAGE \
                | WS_FLAG_CONNECTION \
                | WS_FLAG_HOST \
                | WS_FLAG_SEC_ORIGIN \
                | WS_FLAG_SEC_KEY \
                | WS_FLAG_SEC_VERSION)
#define WS_MD5 (WS_FLAG_HOST \
               | WS_FLAG_CONNECTION \
               | WS_FLAG_SEC_KEY1 \
               | WS_FLAG_SEC_KEY2 \
               | WS_FLAG_UPGRAGE \
               | WS_FLAG_ORIGIN \
               | WS_FLAG_SEC_PROTOCOL)

const char* const WS_FIELD_GET = "get";
const char* const WS_FIELD_HOST = "host";
const char* const WS_FIELD_CONNECTION = "connection";
const char* const WS_FIELD_UPGRADE = "upgrade";
const char* const WS_FIELD_ORIGIN = "origin";
const char* const WS_FIELD_SEC_ORIGIN = "sec-websocket-origin";
const char* const WS_FIELD_SEC_KEY = "sec-websocket-key";
const char* const WS_FIELD_SEC_VERSION = "sec-websocket-version";
const char* const WS_FIELD_SEC_KEY1 = "sec-websocket-key1";
const char* const WS_FIELD_SEC_KEY2 = "sec-websocket-key2";
const char* const WS_FIELD_WS_ORIGIN = "Websocket-Origin";
const char* const WS_FIELD_WS_LOCATION = "websocket-location";
const char* const WS_FIELD_SEC_ACCEPT = "sec-websocket-accept";
const char* const WS_FIELD_SEC_LOCATION = "sec-websocket-location";
const char* const WS_FIELD_SEC_PROTOCOL = "sec-websocket-protocol";

typedef struct _wsconn_frame_t
{
    int32_t flag;
    int32_t version;
    char key[32];
    char key1[32];
    char key2[32];
    char origin[64];
    char protocol[64];
    char host[64];
    char location[64];
}_wsconn_frame_t;

// return 0, parse success
// return -1, parse fail
int32_t _wsconn_parse_request(struct _wsconn_frame_t* frame, const char* data, int32_t sz)
{
    if(!frame || !data || sz < 0 || sz >= WS_FRAME_MAX_SIZE)
        return -1;
    char frame_data[WS_FRAME_MAX_SIZE];
    memcpy(frame_data, data, sz);
    char* delim = "\r\n";
    char head_delim = head_delim;
    char field_delim = ':';
    char* p = NULL, *q = NULL;
    memset(frame, 0, sizeof(*frame));
    p = strtok(frame_data, delim);

    // GET location HTTP/1,1
    if (!p)
        return -1;
    if ((q = strstr(p, &head_delim)) == NULL)
        return -1;
    while (*q++ == head_delim);
    int32_t i = 0;
    while ((frame->location[i++] = *q++) != head_delim && i < sizeof(frame->location));

    // head field
    while ((p = strtok(NULL, delim)) != NULL)
    {
        if ((q = strstr(p, &field_delim)) != NULL)
        {
            *q = '\0';
            if (0 == strcasecmp(p, WS_FIELD_CONNECTION))
            {
                frame->flag |= WS_FLAG_CONNECTION;
                while (*++q == head_delim);
                if (strcasecmp(q, "upgrade"))
                    goto PROTOCOL_FAIL;
            }
            else if (0 == strcasecmp(p, WS_FIELD_UPGRADE))
            {
                frame->flag |= WS_FLAG_UPGRAGE;
                while (*++q == head_delim);
                if (strcasecmp(q, "websocket"))
                    goto PROTOCOL_FAIL;
            }
            else if (0 == strcasecmp(p, WS_FIELD_HOST))
            {
                frame->flag |= WS_FLAG_HOST;
                while (*++q == head_delim);
                snprintf(frame->host, sizeof(frame->host), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_ORIGIN))
            {
                frame->flag |= WS_FLAG_ORIGIN;
                while (*++q == head_delim);
                snprintf(frame->origin, sizeof(frame->origin), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_ORIGIN))
            {
                frame->flag |= WS_FLAG_SEC_ORIGIN;
                while (*++q == head_delim);
                snprintf(frame->origin, sizeof(frame->origin), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY))
            {
                frame->flag |= WS_FLAG_SEC_KEY;
                while (*++q == head_delim);
                snprintf(frame->key, sizeof(frame->key), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_VERSION))
            {
                frame->flag |= WS_FLAG_SEC_VERSION;
                while (*q++ == head_delim);
                frame->version = atoi(q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY1))
            {
                frame->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == head_delim);
                snprintf(frame->key1, sizeof(frame->key1), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY2))
            {
                frame->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == head_delim);
                snprintf(frame->key2, sizeof(frame->key2), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_PROTOCOL))
            {
                frame->flag |= WS_FLAG_SEC_PROTOCOL;
                while (*++q == head_delim);
                snprintf(frame->protocol, sizeof(frame->protocol), "%s", q);
            }
        }
        else
        {
            while (*++p == head_delim);
            snprintf(frame->key, sizeof(frame->key), "%s", q);
        }
    }
    return 0;

PROTOCOL_FAIL:
    printf("parse handshake error:   %s:%s\n", p, q);
    return -1;
}

/*
websocket handshake for flash
    [client request]
        GET /ls HTTP/1.1
        Upgrade: WebSocket
        Connection: Upgrade
        Host: www.xx.com
        Origin: http://www.xx.com
    [server response]
        HTTP/1.1 101 Web Socket Protocol Handshake
        Upgrade: WebSocket
        Connection: Upgrade
        WebSocket-Origin: http://www.xx.com
        WebSocket-Location: ws://www.xx.com/ls
*/
int32_t _wsconn_proc_flash_req(struct wsconn_t* con, struct _wsconn_frame_t* frame)
{
    if (!con || !frame)
        return -1;
    char res[WS_FRAME_MAX_SIZE];
    snprintf(res, sizeof(res), "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
             "Upgrade: WebSocket\r\n"
             "Connection: Upgrade\r\n"
             "WebSocket-Origin: %s\r\n"
             "WebSocket-Location: ws://%s%s\r\n\r\n",
             frame->origin, frame->host, frame->location);
    if(wsconn_send(con, res, strlen(res)) < 0)
        return -1;
    return 0;
}

/*
websocket handshake for version > 13: sha1 + base64 encrypt
    [client request]
        GET /ls HTTP/1.1
        Upgrade: websocket
        Connection: Upgrade
        Host: www.xx.com
        Sec-WebSocket-Origin: http://www.xx.com
        Sec-WebSocket-Key: 2SCVXUeP9cTjV+0mWB8J6A==
        Sec-WebSocket-Version: 13
    [server response]
        HTTP/1.1 101 Switching Protocols
        Upgrade: websocket
        Connection: Upgrade
        Sec-WebSocket-Accept: mLDKNeBNWz6T9SxU+o0Fy/HgeSw=
*/
int32_t _wsconn_proc_sha1_req(struct wsconn_t* con, struct _wsconn_frame_t* frame)
{
    if (!con || !frame)
        return -1;
    char res[WS_FRAME_MAX_SIZE];
    char key[128], sha[128], base64[128];
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", frame->key);
    sha1(sha, key, strnlen(key, sizeof(key)));
    util_base64_encode(base64, sha, strlen(sha));
    snprintf(res, sizeof(res), "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n", res);
    if(wsconn_send(con, res, strlen(res)) < 0)
        return -1;
    return 0;
}

/*
websocket handshake for no version: md5 encrypt
    [client request]
        GET /demo HTTP/1.1
        Host: example.com
        Connection: Upgrade
        Sec-WebSocket-Key2: 12998 5 Y3 1  .P00
        Sec-WebSocket-Protocol: sample
        Upgrade: WebSocket
        Sec-WebSocket-Key1: 4 @1  46546xW%0l 1 5
        Origin: http://example.com
        ^n:ds[4U
    [server response]
        HTTP/1.1 101 WebSocket Protocol Handshake
        Upgrade: WebSocket
        Connection: Upgrade
        Sec-WebSocket-Origin: http://example.com
        Sec-WebSocket-Location: ws://example.com/demo
        Sec-WebSocket-Protocol: sample
        8jKS’y:G*Co,Wxa-
*/
int32_t _wsconn_proc_md5_req(struct wsconn_t* con, struct _wsconn_frame_t* frame)
{
    // TODO:
    return -1;
}

int32_t _wsconn_handshake(struct wsconn_t* con, const char* data, int32_t sz)
{
    _wsconn_frame_t frame;
    int32_t ret;

    ret = _wsconn_parse_request(&frame, data, sz);
    if (ret < 0)
        return -1;

    if (frame.flag & WS_FLASH)
    {
        ret = _wsconn_proc_flash_req(con, &frame);
    }
    else if (frame.flag & WS_SHA1)
    {
        ret = _wsconn_proc_sha1_req(con, &frame);
    }
    else if (frame.flag & WS_MD5)
    {
        ret = _wsconn_proc_md5_req(con, &frame);
    }
    else
    {
        printf("invalid frame flag=%d\n", frame.flag);
        return -1;
    }

    // handshake callback
    if (0 == ret)
    {
        con->build_cb(wsconn_fd(con));
    }
    return ret;
}

int32_t _wsconn_frame(struct wsconn_t* ws, const char* data, int32_t sz)
{
    // TODO: callback connector-read
    return -1;
}


int32_t _wsconn_read(struct handler_t* h)
{
    char* buffer;
    int32_t nread, nwrite, res;
    struct wsconn_t* con = (struct wsconn_t*)h;

    nwrite = connbuffer_write_len(con->read_buf);
    assert(nwrite >= 0);

    /* read buffer full, read callback again */
    if(0 == nwrite)
    {
        if(con->read_cb)
        {
            buffer = connbuffer_read_buffer(con->read_buf);
            nread = connbuffer_read_len(con->read_buf);
            assert(buffer && nread);
            res = con->read_cb(con->h.fd, buffer, nread);
            if(res > 0)
                connbuffer_read_nocopy(con->read_buf, res);
            else if(res < 0)
                return res;
            nwrite = connbuffer_write_len(con->read_buf);
        }
    }

    /* still full, read fail, reactor will close connector */
    if(0 == nwrite)
        return -1;

    /* read socket */
    buffer = connbuffer_write_buffer(con->read_buf);
    res = sock_read(con->h.fd, buffer, nwrite);
    if(res < 0)
    {
        /* can't read now */
        if(ERR_EAGAIN == ERRNO || ERR_EWOULDBLOCK == ERRNO || ERR_EINTR == ERRNO)
            return 0;
        else
        {
            printf("fd[%d] read errno=%d\n", con->h.fd, ERRNO);
            return -1;
        }
    }
    else if(0 == res)
    {
        // printf("fd[%d] peer close\n", con->h.fd);
        return -1;
    }
    else
    {
        connbuffer_write_nocopy(con->read_buf, res);
        buffer = connbuffer_read_buffer(con->read_buf);
        nread = connbuffer_read_len(con->read_buf);
        assert(buffer && nread);
        int32_t from = 0;
        while(from < nread - 4)
        {
            if(buffer[from] == '\r' && buffer[from + 1] == '\n'
               && buffer[from + 2] == '\r'&& buffer[from + 3] == '\n')
            {
                if (0 == wsconn_established(con))
                    res = _wsconn_frame(con, buffer, from + 4);
                else
                    res = _wsconn_handshake(con, buffer, from + 4);
                break;
            }
            from ++;
        }
        if (res < 0)
            return res;
        connbuffer_read_nocopy(con->read_buf, from + 4);
    }
    return 0;
}

int32_t _wsconn_write(struct handler_t* h)
{
    char* buffer;
    int32_t nwrite, res;
    struct wsconn_t* con = (struct wsconn_t*)h;

    nwrite = connbuffer_read_len(con->write_buf);
    if(nwrite <= 0)
        return 0;
    buffer = connbuffer_read_buffer(con->write_buf);
    res = sock_write(con->h.fd, buffer, nwrite);
    if(res < 0)
    {
        /* can't write now */
        if(ERR_EAGAIN == ERRNO || ERR_EWOULDBLOCK == ERRNO || ERR_EINTR == ERRNO)
            return 0;
        else
        {
            printf("write %d errno=%d\n", con->h.fd, ERRNO);
            return -1;
        }
    }
    else if(0 == res)
    {
        return -1;
    }
    else
    {
        connbuffer_read_nocopy(con->write_buf, res);
        if(res == nwrite)
            reactor_modify(con->r, &con->h, EVENT_IN);
    }
    return 0;
}

int32_t _wsconn_close(struct handler_t* h)
{
    struct wsconn_t* con = (struct wsconn_t*)h;
    if(con->close_cb)
        con->close_cb(con->h.fd);
    return 0;
}

struct wsconn_t* wsconn_init(struct reactor_t* r,
                             wsconn_build_func build_cb,
                             wsconn_read_func read_cb,
                             wsconn_close_func close_cb,
                             struct connbuffer_t* read_buf,
                             struct connbuffer_t* write_buf,
                             int32_t fd)
{
    struct wsconn_t* con;
    if(!r || fd < 0 || !read_buf || !write_buf)
        return NULL;
    con = (struct wsconn_t*)MALLOC(sizeof(struct wsconn_t));
    if(!con)
        return NULL;
    con->h.fd = fd;
    con->h.in_func = _wsconn_read;
    con->h.out_func = _wsconn_write;
    con->h.close_func = _wsconn_close;
    con->r = r;
    con->read_buf = read_buf;
    con->write_buf = write_buf;
    con->build_cb = build_cb;
    con->read_cb = read_cb;
    con->close_cb = close_cb;
    return con;
}

int32_t wsconn_release(struct wsconn_t* con)
{
    if(con)
    {
        wsconn_stop(con);
        FREE(con);
    }
    return 0;
}

int32_t wsconn_fd(struct wsconn_t* con)
{
    if(con)
        return con->h.fd;
    return -1;
}

int32_t wsconn_start(struct wsconn_t* con)
{
    if(!con || con->h.fd < 0)
        return -1;
    sock_set_nonblock(con->h.fd);
    sock_set_nodelay(con->h.fd);
    return reactor_register(con->r, &con->h, EVENT_IN);
}

/*
*    return < 0 success
*    return >=0, send bytes, maybe < buflen, some bytes full discard
*/
int32_t wsconn_send(struct wsconn_t* con, const char* buffer, int32_t buflen)
{
    int32_t nwrite, res;
    if(!con || !buffer || buflen < 0)
        return -1;
    nwrite = connbuffer_write_len(con->write_buf);
    res = connbuffer_write(con->write_buf, buffer, buflen);

    /* add out events (no buffer before send) */
    if (res == connbuffer_read_len(con->write_buf))
        reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return res;
}

int32_t wsconn_stop(struct wsconn_t* con)
{
    if(!con || con->h.fd < 0)
        return -1;
    reactor_unregister(con->r, &con->h);
    sock_close(con->h.fd);
    con->h.fd = -1;
    return 0;
}

int32_t wsconn_established(struct wsconn_t* con)
{
    if(con)
        return con->status == WS_ESTABLISHED ? 0 : -1;
    return -1;
}


