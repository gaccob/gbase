#include "net/wsconn.h"
#include "ds/sha1.h"
#include "ds/md5.h"
#include "os/util.h"

typedef struct wsconn_t
{
    struct handler_t h;
    struct reactor_t* r;
    struct connbuffer_t* read_buf;
    struct connbuffer_t* real_read_buf;
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

#define WS_HANDSHAKE_MAX_SIZE 1024

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

typedef struct wsconn_request_t
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
}wsconn_request_t;

// return 0, parse success
// return -1, parse fail
int32_t _wsconn_parse_request(struct wsconn_request_t* request, const char* data, int32_t sz)
{
    if(!request || !data || sz < 0 || sz >= WS_HANDSHAKE_MAX_SIZE)
        return -1;
    char request_data[WS_HANDSHAKE_MAX_SIZE];
    memcpy(request_data, data, sz);
    char* delim = "\r\n";
    char head_delim = head_delim;
    char field_delim = ':';
    char* p = NULL, *q = NULL;
    memset(request, 0, sizeof(*request));
    p = strtok(request_data, delim);

    // GET location HTTP/1,1
    if (!p)
        return -1;
    if ((q = strstr(p, &head_delim)) == NULL)
        return -1;
    while (*q++ == head_delim);
    int32_t i = 0;
    while ((request->location[i++] = *q++) != head_delim && i < sizeof(request->location));

    // head field
    while ((p = strtok(NULL, delim)) != NULL)
    {
        if ((q = strstr(p, &field_delim)) != NULL)
        {
            *q = '\0';
            if (0 == strcasecmp(p, WS_FIELD_CONNECTION))
            {
                request->flag |= WS_FLAG_CONNECTION;
                while (*++q == head_delim);
                if (strcasecmp(q, "upgrade"))
                    goto PROTOCOL_FAIL;
            }
            else if (0 == strcasecmp(p, WS_FIELD_UPGRADE))
            {
                request->flag |= WS_FLAG_UPGRAGE;
                while (*++q == head_delim);
                if (strcasecmp(q, "websocket"))
                    goto PROTOCOL_FAIL;
            }
            else if (0 == strcasecmp(p, WS_FIELD_HOST))
            {
                request->flag |= WS_FLAG_HOST;
                while (*++q == head_delim);
                snprintf(request->host, sizeof(request->host), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_ORIGIN))
            {
                request->flag |= WS_FLAG_ORIGIN;
                while (*++q == head_delim);
                snprintf(request->origin, sizeof(request->origin), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_ORIGIN))
            {
                request->flag |= WS_FLAG_SEC_ORIGIN;
                while (*++q == head_delim);
                snprintf(request->origin, sizeof(request->origin), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY))
            {
                request->flag |= WS_FLAG_SEC_KEY;
                while (*++q == head_delim);
                snprintf(request->key, sizeof(request->key), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_VERSION))
            {
                request->flag |= WS_FLAG_SEC_VERSION;
                while (*q++ == head_delim);
                request->version = atoi(q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY1))
            {
                request->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == head_delim);
                snprintf(request->key1, sizeof(request->key1), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY2))
            {
                request->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == head_delim);
                snprintf(request->key2, sizeof(request->key2), "%s", q);
            }
            else if (0 == strcasecmp(p, WS_FIELD_SEC_PROTOCOL))
            {
                request->flag |= WS_FLAG_SEC_PROTOCOL;
                while (*++q == head_delim);
                snprintf(request->protocol, sizeof(request->protocol), "%s", q);
            }
        }
        else
        {
            while (*++p == head_delim);
            snprintf(request->key, sizeof(request->key), "%s", q);
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
int32_t _wsconn_proc_flash_req(struct wsconn_t* con, struct wsconn_request_t* request)
{
    if (!con || !request)
        return -1;
    char res[WS_HANDSHAKE_MAX_SIZE];
    snprintf(res, sizeof(res), "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
             "Upgrade: WebSocket\r\n"
             "Connection: Upgrade\r\n"
             "WebSocket-Origin: %s\r\n"
             "WebSocket-Location: ws://%s%s\r\n\r\n",
             request->origin, request->host, request->location);
    int32_t nwrite = connbuffer_write_len(con->write_buf);
    int32_t nres = (int32_t)strlen(res);
    if (nwrite < nres)
        return -1;
    connbuffer_write(con->write_buf, res, nres);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
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
int32_t _wsconn_proc_sha1_req(struct wsconn_t* con, struct wsconn_request_t* request)
{
    if (!con || !request)
        return -1;
    char res[WS_HANDSHAKE_MAX_SIZE];
    char key[128], sha[128], base64[128];
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", request->key);
    sha1(sha, key, strnlen(key, sizeof(key)));
    util_base64_encode(base64, sha, strlen(sha));
    snprintf(res, sizeof(res), "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n", res);
    int32_t nwrite = connbuffer_write_len(con->write_buf);
    int32_t nres = (int32_t)strlen(res);
    if (nwrite < nres)
        return -1;
    connbuffer_write(con->write_buf, res, nres);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
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
int32_t _wsconn_proc_md5_req(struct wsconn_t* con, struct wsconn_request_t* request)
{
    uint8_t chrkey1[4];
    uint8_t chrkey2[4];
    uint32_t k1 = util_str2int(request->key1);
    uint32_t k2 = util_str2int(request->key2);
    uint8_t i;
    for (i = 0; i < 4; i++)
        chrkey1[i] = k1 << (8 * i) >> (8 * 3);
    for (i = 0; i < 4; i++)
        chrkey2[i] = k2 << (8 * i) >> (8 * 3);
    uint8_t encrypt[17];
    uint8_t keys[16];
    memcpy(keys, chrkey1, 4);
    memcpy(&keys[4], chrkey2, 4);
    memcpy(&keys[8], request->key, 8);
    md5(encrypt, keys, sizeof (keys));
    encrypt[16] = 0;
    char res[WS_HANDSHAKE_MAX_SIZE];
    snprintf(res, sizeof(res), "HTTP/1.1 101 WebSocket Protocol Handshake"
             "Upgrade: WebSocket\r\n"
             "Connection: Upgrade\r\n"
             "Sec-WebSocket-Origin: %s\r\n"
             "Sec-WebSocket-Location: ws://%s%s\r\n"
             "Sec-WebSocket-Protocol: %s\r\n"
             "%s\r\n\r\n",
             request->origin,
             request->host,
             request->location,
             request->protocol,
             encrypt);
    int32_t nwrite = connbuffer_write_len(con->write_buf);
    int32_t nres = (int32_t)strlen(res);
    if (nwrite < nres)
        return -1;
    connbuffer_write(con->write_buf, res, nres);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

// return processed bytes
// return < 0 means processed fail, reactor will close connection
int32_t _wsconn_handshake(struct wsconn_t* con, char* data, int32_t sz)
{
    wsconn_request_t request;
    int32_t ret;

    ret = _wsconn_parse_request(&request, data, sz);
    if (ret < 0)
        return -1;

    if (request.flag & WS_FLASH)
    {
        ret = _wsconn_proc_flash_req(con, &request);
    }
    else if (request.flag & WS_SHA1)
    {
        ret = _wsconn_proc_sha1_req(con, &request);
    }
    else if (request.flag & WS_MD5)
    {
        ret = _wsconn_proc_md5_req(con, &request);
    }
    else
    {
        printf("invalid request flag=%d\n", request.flag);
        return -1;
    }

    // handshake callback
    if (0 == ret)
    {
        con->build_cb(wsconn_fd(con));
        return sz;
    }
    return -1;
}

typedef struct wsconn_frame_t
{
    int8_t is_fin;
    int8_t is_masked;
    int8_t opcode;
    int8_t mask_shift;
    int8_t mask[4];
    int8_t payload_shift;
    uint64_t payload_len;
}wsconn_frame_t;

/*
   0                   1                   2                   3
   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-------+-+-------------+-------------------------------+
   |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
   |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
   |N|V|V|V|       |S|             |   (if payload len==126/127)   |
   | |1|2|3|       |K|             |                               |
   +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
   |     Extended payload length continued, if payload len == 127  |
   + - - - - - - - - - - - - - - - +-------------------------------+
   |                               |Masking-key, if MASK set to 1  |
   +-------------------------------+-------------------------------+
   | Masking-key (continued)       |          Payload Data         |
   +-------------------------------- - - - - - - - - - - - - - - - +
   :                     Payload Data continued ...                :
   + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
   |                     Payload Data continued ...                |
   +---------------------------------------------------------------+

    opcode:
         *  %x0 denotes a continuation frame
         *  %x1 denotes a text frame
         *  %x2 denotes a binary frame
         *  %x3-7 are reserved for further non-control frames
         *  %x8 denotes a connection close
         *  %x9 denotes a ping
         *  %xA denotes a pong
         *  %xB-F are reserved for further control frames

   Payload length:  7 bits, 7+16 bits, or 7+64 bits

   Masking-key:  0 or 4 bytes

*/
// return processed bytes
// return < 0 means fail, reactor will close connection
int32_t _wsconn_frame(struct wsconn_t* con, char* data, int32_t sz)
{
    if (sz < 2)
        return 0;

    int32_t i, res, nread;
    char* buffer;
    wsconn_frame_t frame;
    memset(&frame, 0, sizeof(frame));
    frame.is_fin = data[0] & 0x80;
    frame.opcode = data[0] & 0x0f;

    /* only support text mode now */
    if (frame.opcode != 0x01)
        return -1;

    frame.is_masked = data[1] & 0x80;
    frame.payload_len = data[1] & 0x7f;
    frame.mask_shift = 2;
    if (frame.payload_len == 126)
    {
        /* not enough data */
        if (sz < 2 + 2) return 0;
        /* change byte order */
        uint16_t b1, b2;
        b1 = (uint8_t)data[2];
        b2 = (uint8_t)data[3];
#if defined(OS_LITTLE_ENDIAN)
        frame.payload_len = ((b1 << 8) | b2);
#elif defined(OS_BIG_ENDIAN)
        frame.payload_len = ((b2 << 8) | b1);
#endif
        frame.mask_shift = 4;
    }
    else if (frame.payload_len == 127)
    {
        /* not enough data */
        if (sz < 2 + 8) return 0;
        /* change byte order */
        uint64_t b1,b2,b3,b4,b5,b6,b7,b8;
        b1 = (uint8_t)data[2];
        b2 = (uint8_t)data[3];
        b3 = (uint8_t)data[4];
        b4 = (uint8_t)data[5];
        b5 = (uint8_t)data[6];
        b6 = (uint8_t)data[7];
        b7 = (uint8_t)data[8];
        b8 = (uint8_t)data[9];
#if defined(OS_LITTLE_ENDIAN)
        frame.payload_len = (b1 << 56 | b2 << 48 | b3 << 40 | b4 << 32 | b5 << 24 | b6 << 16 | b7 << 8 | b8);
#elif defined(OS_BIG_ENDIAN)
        frame.payload_len = (b1 | b2 << 8 | b3 << 16 | b4 << 24 | b5 << 32 | b6 << 40 | b7 << 48 | b8 << 56);
#endif
        frame.mask_shift = 10;
    }

    /*
       from RFC6455 (http://tools.ietf.org/html/rfc6455):
       The payload length is the length of the "Extension data" + the length of the "Application data"
    */
    if (sz < frame.payload_len + 2)
        return 0;

    if (frame.is_masked)
    {
        frame.mask[0] = data[frame.mask_shift];
        frame.mask[1] = data[frame.mask_shift + 1];
        frame.mask[2] = data[frame.mask_shift + 2];
        frame.mask[3] = data[frame.mask_shift + 3];
    }

    if (frame.is_masked)
        frame.payload_shift = frame.mask_shift + 4;
    else
        frame.payload_shift = frame.mask_shift;

    /* masked release */
    if (frame.is_masked)
    {
        for (i=frame.payload_shift; i<2+frame.payload_len; i++)
            data[i] = data[i] ^ frame.mask[(i-frame.payload_shift) % 4];
    }

    /* write to buffer */
    res = connbuffer_write(con->real_read_buf,
                                   &data[frame.payload_shift],
                                   frame.payload_len + 2 - frame.payload_shift);
    if (res < 0)
        return -1;

    /* read callback */
    buffer = connbuffer_read_buffer(con->real_read_buf);
    nread = connbuffer_read_len(con->real_read_buf);
    assert(buffer && nread);
    res = con->read_cb(con->h.fd, buffer, nread);
    if (res > 0)
        connbuffer_read_nocopy(con->real_read_buf, res);
    else
        return res;

    /* return processed bytes */
    return frame.payload_len + 2;
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

        if (0 != wsconn_established(con))
        {
            int32_t from = 0;
            while(from < nread - 4)
            {
                if(buffer[from] == '\r'
                    && buffer[from + 1] == '\n'
                    && buffer[from + 2] == '\r'
                    && buffer[from + 3] == '\n')
                {
                    res = _wsconn_handshake(con, buffer, from + 4);
                    break;
                }
                from ++;
            }

        }
        else
        {
            res = _wsconn_handshake(con, buffer, nread);
        }
        if (res < 0)
            return res;
        connbuffer_read_nocopy(con->read_buf, res);
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
    int8_t finish_data[4];
    finish_data[0] = 0x08;
    finish_data[1] = 0x02;
    finish_data[2] = 0x03;
    finish_data[3] = 0xF1;
    wsconn_send(con, (char*)finish_data, 4);
    if(con->close_cb)
        con->close_cb(con->h.fd);
    return 0;
}

struct wsconn_t* wsconn_init(struct reactor_t* r,
                             wsconn_build_func build_cb,
                             wsconn_read_func read_cb,
                             wsconn_close_func close_cb,
                             struct connbuffer_t* read_buf,
                             struct connbuffer_t* real_read_buf,
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
    con->real_read_buf = real_read_buf;
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
*    return = 0 success
*    return < 0, fail maybe full
*/
int32_t wsconn_send(struct wsconn_t* con, const char* buffer, int32_t buflen)
{
    int32_t nwrite;
    int8_t head, sz;
    if(!con || !buffer || buflen < 0)
        return -1;

    if (0 != wsconn_established(con))
        return -1;

    nwrite = connbuffer_write_len(con->write_buf);
    head = 0x81; /*text mode*/
    if (buflen < 126)
    {
        if (nwrite < buflen + 2)
            return -1;
        connbuffer_write(con->write_buf, (char*)&head, 1);
        sz = buflen;
        connbuffer_write(con->write_buf, (char*)&sz, 1);
        connbuffer_write(con->write_buf, buffer, buflen);
    }
    else if (buflen <= 65535)
    {
        if (nwrite < buflen + 4)
            return -1;
        int8_t c = 126;
        connbuffer_write(con->write_buf, (char*)&c, 1);
        int16_t len = 2 + buflen;
#if defined(OS_LITTLE_ENDIAN)
        connbuffer_write(con->write_buf, (char*)&len + 1, 1);
        connbuffer_write(con->write_buf, (char*)&len, 1);
#elif defined(OS_BIG_ENDIAN)
        connbuffer_write(con->write_buf, (char*)&len, 2);
#endif
        connbuffer_write(con->write_buf, buffer, buflen);
    }
    else
    {
        if (nwrite < buflen + 10)
            return -1;
        int8_t c = 127;
        connbuffer_write(con->write_buf, (char*)&c, 1);
        int64_t len = 10 + buflen;
#if defined(OS_LITTLE_ENDIAN)
        connbuffer_write(con->write_buf, (char*)&len + 7, 1);
        connbuffer_write(con->write_buf, (char*)&len + 6, 1);
        connbuffer_write(con->write_buf, (char*)&len + 5, 1);
        connbuffer_write(con->write_buf, (char*)&len + 4, 1);
        connbuffer_write(con->write_buf, (char*)&len + 3, 1);
        connbuffer_write(con->write_buf, (char*)&len + 2, 1);
        connbuffer_write(con->write_buf, (char*)&len + 1, 1);
        connbuffer_write(con->write_buf, (char*)&len, 1);
#elif defined(OS_BIG_ENDIAN)
        connbuffer_write(con->write_buf, (char*)&len, 8);
#endif
        connbuffer_write(con->write_buf, buffer, buflen);
    }

    /* add out events (no buffer before send) */
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
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


