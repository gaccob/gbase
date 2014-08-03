#include "util/sha1.h"
#include "util/md5.h"
#include "util/base64.h"
#include "util/util_str.h"
#include "wsconn.h"

#define WSCONN_BUFFER_SIZE (64 * 1024)

struct wsconn_t {
    handler_t h;
    reactor_t* r;

    // buffer & flag
    int8_t flag_rbuf : 1;
    int8_t flag_rrbuf : 1;
    int8_t flag_wbuf : 1;
    buffer_t* rbuf;
    buffer_t* rrbuf;
    buffer_t* wbuf;

    // callback function
    wsconn_build_func on_build;
    void* build_arg;
    wsconn_read_func on_read;
    void* read_arg;
    wsconn_close_func on_close;
    void* close_arg;

    int8_t status;
};

enum {
    WS_INIT,
    WS_ESTABLISHED,
    WS_FINI,
};

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

#define WS_SHA1_EX (WS_FLAG_UPGRAGE \
                   | WS_FLAG_CONNECTION \
                   | WS_FLAG_HOST \
                   | WS_FLAG_ORIGIN \
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

typedef struct wsconn_request_t {
    int flag;
    int version;
    char key[32];
    char key1[32];
    char key2[32];
    char origin[64];
    char protocol[64];
    char host[64];
    char location[64];
} request_t;

// return 0, parse success
// return -1, parse fail
static int
_wsconn_parse_request(request_t* request, const char* data, int sz) {
    char request_data[WS_HANDSHAKE_MAX_SIZE];
    char* delim = "\r\n";
    char* head_delim = " ";
    char* field_delim = ":";
    char* p = NULL, *q = NULL;
    int i = 0;

    memcpy(request_data, data, sz);
    if (!request || !data || sz < 0 || sz >= WS_HANDSHAKE_MAX_SIZE)
        return -1;
    memcpy(request_data, data, sz);
    memset(request, 0, sizeof(*request));
    p = strtok(request_data, delim);

    // GET location HTTP/1,1
    if (!p) return -1;
    if ((q = strstr(p, head_delim)) == NULL) return -1;

    q++;
    while ((request->location[i++] = *q++) != *head_delim
        && i < sizeof(request->location));
    request->location[i-1] = '\0';

    // head field
    while ((p = strtok(NULL, delim)) != NULL) {
        if ((q = strstr(p, field_delim)) != NULL) {
            *q = '\0';
            if (0 == strcasecmp(p, WS_FIELD_CONNECTION)) {
                request->flag |= WS_FLAG_CONNECTION;
                while (*++q == *head_delim);
                if (strcasecmp(q, "upgrade"))
                    goto PROTOCOL_FAIL;
            } else if (0 == strcasecmp(p, WS_FIELD_UPGRADE)) {
                request->flag |= WS_FLAG_UPGRAGE;
                while (*++q == *head_delim);
                if (strcasecmp(q, "websocket"))
                    goto PROTOCOL_FAIL;
            } else if (0 == strcasecmp(p, WS_FIELD_HOST)) {
                request->flag |= WS_FLAG_HOST;
                while (*++q == *head_delim);
                snprintf(request->host, sizeof(request->host), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_ORIGIN)) {
                request->flag |= WS_FLAG_ORIGIN;
                while (*++q == *head_delim);
                snprintf(request->origin, sizeof(request->origin), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_ORIGIN)) {
                request->flag |= WS_FLAG_SEC_ORIGIN;
                while (*++q == *head_delim);
                snprintf(request->origin, sizeof(request->origin), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY)) {
                request->flag |= WS_FLAG_SEC_KEY;
                while (*++q == *head_delim);
                snprintf(request->key, sizeof(request->key), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_VERSION)) {
                request->flag |= WS_FLAG_SEC_VERSION;
                while (*q++ == *head_delim);
                request->version = atoi(q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY1)) {
                request->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == *head_delim);
                snprintf(request->key1, sizeof(request->key1), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_KEY2)) {
                request->flag |= WS_FLAG_SEC_KEY1;
                while (*++q == *head_delim);
                snprintf(request->key2, sizeof(request->key2), "%s", q);
            } else if (0 == strcasecmp(p, WS_FIELD_SEC_PROTOCOL)) {
                request->flag |= WS_FLAG_SEC_PROTOCOL;
                while (*++q == *head_delim);
                snprintf(request->protocol, sizeof(request->protocol), "%s", q);
            }
        } else if (request->key[0] == 0) {
            while (*++p == *head_delim);
            snprintf(request->key, sizeof(request->key), "%s", q);
        }
    }
    printf("websocket handshake request:\n"
            "   version: %d\n"
            "   key: %s\n"
            "   key1: %s\n"
            "   key2: %s\n"
            "   origin: %s\n"
            "   protocol: %s\n"
            "   host: %s\n"
            "   location: %s\n",
            request->version,
            request->key,
            request->key1,
            request->key2,
            request->origin,
            request->protocol,
            request->host,
            request->location);
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
static int
_wsconn_proc_flash_req(wsconn_t* con, request_t* request) {
    if (!con || !request)
        return -1;
    char res[WS_HANDSHAKE_MAX_SIZE];
    snprintf(res, sizeof(res), "HTTP/1.1 101 Web Socket Protocol Handshake\r\n"
             "Upgrade: WebSocket\r\n"
             "Connection: Upgrade\r\n"
             "WebSocket-Origin: %s\r\n"
             "WebSocket-Location: ws://%s%s\r\n\r\n",
             request->origin, request->host, request->location);
    int nwrite = buffer_write_len(con->wbuf);
    int nres = (int)strlen(res);
    if (nwrite < nres)
        return -1;
    buffer_write(con->wbuf, res, nres);
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
static int
_wsconn_proc_sha1_req(wsconn_t* con, request_t* request) {
    if (!con || !request)
        return -1;

    char key[64], sha[128], base64_code[128];
    snprintf(key, sizeof(key), "%s258EAFA5-E914-47DA-95CA-C5AB0DC85B11", request->key);
    memset(sha, 0, sizeof(sha));
    sha1(sha, key, strlen(key) * 8);
    base64_encode(base64_code, sha, strlen(sha));

    char res[WS_HANDSHAKE_MAX_SIZE];
    snprintf(res, sizeof(res), "HTTP/1.1 101 Switching Protocols\r\n"
            "Upgrade: websocket\r\n"
            "Connection: Upgrade\r\n"
            "Sec-WebSocket-Accept: %s\r\n\r\n", base64_code);

    int nwrite = buffer_write_len(con->wbuf);
    int nres = (int)strlen(res);
    if (nwrite < nres)
        return -1;
    buffer_write(con->wbuf, res, nres);
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
static int
_wsconn_proc_md5_req(wsconn_t* con, request_t* request) {
    uint32_t k1 = util_str2int(request->key1);
    uint32_t k2 = util_str2int(request->key2);

    uint8_t chrkey1[4];
    for (int i = 0; i < 4; i++)
        chrkey1[i] = k1 << (8 * i) >> (8 * 3);

    uint8_t chrkey2[4];
    for (int i = 0; i < 4; i++)
        chrkey2[i] = k2 << (8 * i) >> (8 * 3);

    uint8_t keys[16];
    memcpy(keys, chrkey1, 4);
    memcpy(&keys[4], chrkey2, 4);
    memcpy(&keys[8], request->key, 8);

    uint8_t encrypt[17];
    md5(encrypt, keys, sizeof(keys));
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
    int nwrite = buffer_write_len(con->wbuf);
    int nres = (int)strlen(res);
    if (nwrite < nres)
        return -1;
    buffer_write(con->wbuf, res, nres);
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

// return processed bytes
// return < 0 means processed fail, reactor will close connection
static int
_wsconn_handshake(wsconn_t* con, char* data, int sz) {
    request_t request;
    int ret = _wsconn_parse_request(&request, data, sz);
    if (ret < 0)
        return -1;

    if ((request.flag & WS_MD5) == WS_MD5) {
        ret = _wsconn_proc_md5_req(con, &request);
    } else if (((request.flag & WS_SHA1) == WS_SHA1)
        || ((request.flag & WS_SHA1_EX) == WS_SHA1_EX)) {
        ret = _wsconn_proc_sha1_req(con, &request);
    } else if ((request.flag & WS_FLASH) == WS_FLASH) {
        ret = _wsconn_proc_flash_req(con, &request);
    } else {
        printf("invalid request flag=%d\n", request.flag);
        return -1;
    }

    // handshake callback
    if (0 == ret) {
        con->status = WS_ESTABLISHED;
        if (con->on_build) {
            con->on_build(wsconn_fd(con), con->build_arg);
        }
        return sz;
    }
    return -1;
}

typedef struct wsconn_frame_t {
    int8_t is_fin;
    int8_t is_masked;
    int8_t opcode;
    int8_t mask_shift;
    int8_t mask[4];
    int8_t payload_shift;
    uint64_t payload_len;
} frame_t;

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
static int
_wsconn_frame(wsconn_t* con, char* data, int sz) {
    int i, res, nread;
    char* buffer;
    frame_t frame;
    uint16_t c1, c2;
    uint64_t b1,b2,b3,b4,b5,b6,b7,b8;

    if (sz < 2) return 0;
    memset(&frame, 0, sizeof(frame));
    frame.is_fin = data[0] & 0x80;
    frame.opcode = data[0] & 0x0f;

    // only support text mode now
    if (frame.opcode != 0x01) return -1;

    frame.is_masked = data[1] & 0x80;
    frame.payload_len = data[1] & 0x7f;
    frame.mask_shift = 2;
    if (frame.payload_len == 126) {
        // not enough data
        if (sz < 2 + 2) return 0;
        // change byte order
        c1 = (uint8_t)data[2];
        c2 = (uint8_t)data[3];
#if defined(OS_LITTLE_ENDIAN)
        frame.payload_len = ((c1 << 8) | c2);
#elif defined(OS_BIG_ENDIAN)
        frame.payload_len = ((c2 << 8) | c1);
#endif
        frame.mask_shift = 4;

    } else if (frame.payload_len == 127) {
        // not enough data
        if (sz < 2 + 8) return 0;
        // change byte order
        b1 = (uint8_t)data[2];
        b2 = (uint8_t)data[3];
        b3 = (uint8_t)data[4];
        b4 = (uint8_t)data[5];
        b5 = (uint8_t)data[6];
        b6 = (uint8_t)data[7];
        b7 = (uint8_t)data[8];
        b8 = (uint8_t)data[9];
#if defined(OS_LITTLE_ENDIAN)
        frame.payload_len = (b1 << 56 | b2 << 48 | b3 << 40
            | b4 << 32 | b5 << 24 | b6 << 16 | b7 << 8 | b8);
#elif defined(OS_BIG_ENDIAN)
        frame.payload_len = (b1 | b2 << 8 | b3 << 16 | b4 << 24
            | b5 << 32 | b6 << 40 | b7 << 48 | b8 << 56);
#endif
        frame.mask_shift = 10;
    }

    if (frame.is_masked) {
        if (sz < frame.payload_len + frame.mask_shift + 4)
            return 0;
        frame.mask[0] = data[frame.mask_shift];
        frame.mask[1] = data[frame.mask_shift + 1];
        frame.mask[2] = data[frame.mask_shift + 2];
        frame.mask[3] = data[frame.mask_shift + 3];
    } else if (sz < frame.payload_len + frame.mask_shift) {
        return 0;
    }

    if (frame.is_masked) {
        frame.payload_shift = frame.mask_shift + 4;
    } else {
        frame.payload_shift = frame.mask_shift;
    }

    // masked release
    if (frame.is_masked) {
        for (i = 0; i < frame.payload_len; i++)
            data[i + frame.payload_shift] = data[i + frame.payload_shift] ^ frame.mask[i % 4];
    }

    // write to buffer
    res = buffer_write(con->rrbuf, &data[frame.payload_shift], (int)frame.payload_len);
    if (res < 0) return -1;

    // read callback
    buffer = buffer_read_buffer(con->rrbuf);
    nread = buffer_read_len(con->rrbuf);
    assert(buffer && nread);
    if (con->on_read) {
        res = con->on_read(con->h.fd, con->read_arg, buffer, nread);
    }
    if (res > 0) {
        buffer_read_nocopy(con->rrbuf, res);
    } else {
        return res;
    }

    // return processed bytes
    return (int)frame.payload_len + frame.mask_shift + (frame.is_masked ? 4 : 0);
}

static int
_wsconn_read(struct handler_t* h) {
    char* buffer;
    int nread, nwrite, res;
    wsconn_t* con = (wsconn_t*)h;
    nwrite = buffer_write_len(con->rbuf);
    assert(nwrite >= 0);

    // read buffer, fail
    if (0 == nwrite) {
        printf("fd[%d] read buffer full fail\n", con->h.fd);
        return -1;
    }

    // still full, read fail, reactor will close connector
    if (0 == nwrite) return -1;

    // read socket
    buffer = buffer_write_buffer(con->rbuf);
    res = read(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't read now
        if (EAGAIN == errno || EINTR == errno) {
            return 0;
        } else {
            printf("fd[%d] read errno=%d\n", con->h.fd, errno);
            return -errno;
        }
    } else if (0 == res) {
        return -1;
    } else {
        buffer_write_nocopy(con->rbuf, res);
        buffer = buffer_read_buffer(con->rbuf);
        nread = buffer_read_len(con->rbuf);
        assert(buffer && nread);
        if (0 != wsconn_established(con)) {
            int from = 0;
            while (from <= nread - 4) {
                if (buffer[from] == '\r'
                    && buffer[from + 1] == '\n'
                    && buffer[from + 2] == '\r'
                    && buffer[from + 3] == '\n') {
                    res = _wsconn_handshake(con, buffer, from + 4);
                    break;
                }
                from ++;
            }
        } else {
            res = _wsconn_frame(con, buffer, nread);
        }
        if (res < 0) return res;
        buffer_read_nocopy(con->rbuf, res);
    }
    return 0;
}

static int
_wsconn_write(struct handler_t* h) {
    char* buffer;
    int nwrite, res;
    wsconn_t* con = (wsconn_t*)h;
    nwrite = buffer_read_len(con->wbuf);
    if (nwrite <= 0) return 0;
    buffer = buffer_read_buffer(con->wbuf);
    res = write(con->h.fd, buffer, nwrite);
    if (res < 0) {
        // can't write now
        if (EAGAIN == errno || EINTR == errno) {
            return 0;
        } else {
            printf("write %d errno=%d\n", con->h.fd, errno);
            return -errno;
        }
    } else if (0 == res) {
        return -1;
    } else {
        buffer_read_nocopy(con->wbuf, res);
        if (res == nwrite)
            reactor_modify(con->r, &con->h, EVENT_IN);
    }
    return 0;
}

static int
_wsconn_close(struct handler_t* h) {
    wsconn_t* con = (wsconn_t*)h;
    if (con->on_close) {
        con->on_close(con->h.fd, con->close_arg);
    }
    return 0;
}

wsconn_t*
wsconn_create(reactor_t* r) {
    wsconn_t* con;
    if (!r)
        return NULL;
    con = (wsconn_t*)MALLOC(sizeof(wsconn_t));
    if (!con) return NULL;

    memset(con, 0, sizeof(wsconn_t));
    con->h.fd = INVALID_SOCK;
    con->h.in_func = _wsconn_read;
    con->h.out_func = _wsconn_write;
    con->h.close_func = _wsconn_close;
    con->r = r;
    con->status = WS_INIT;
    return con;
}

inline int
wsconn_set_buffer(wsconn_t* con,
                  buffer_t* rbuf,
                  buffer_t* wbuf,
                  buffer_t* rrbuf) {
    if (!con || con->rbuf || con->rbuf || con->wbuf)
        return -1;
    con->rbuf = rbuf;
    con->wbuf = wbuf;
    con->rrbuf = rrbuf;
    return 0;
}

inline void
wsconn_set_build_func(wsconn_t* con, wsconn_build_func on_build, void* arg) {
    if (con) {
        con->on_build = on_build;
        con->build_arg = arg;
    }
}

inline void
wsconn_set_read_func(wsconn_t* con, wsconn_read_func on_read, void* arg) {
    if (con) {
        con->on_read = on_read;
        con->read_arg = arg;
    }
}

inline void
wsconn_set_close_func(wsconn_t* con, wsconn_close_func on_close, void* arg) {
    if (con) {
        con->on_close = on_close;
        con->close_arg = arg;
    }
}

int
wsconn_release(wsconn_t* con) {
    if (con) {
        wsconn_stop(con);
        if (con->flag_rbuf) {
            buffer_release(con->rbuf);
        }
        if (con->flag_rrbuf) {
            buffer_release(con->rrbuf);
        }
        if (con->flag_wbuf) {
            buffer_release(con->wbuf);
        }
        FREE(con);
    }
    return 0;
}

inline sock_t
wsconn_fd(wsconn_t* con) {
    return con ? con->h.fd : INVALID_SOCK;
}

inline void
wsconn_set_fd(wsconn_t* con, sock_t fd) {
    if (con) con->h.fd = fd;
}

int
wsconn_start(wsconn_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;

    sock_set_nonblock(con->h.fd);
    sock_set_nodelay(con->h.fd);

    // create default buffer if not exist
    if (!con->rbuf) {
        con->rbuf = buffer_create(WSCONN_BUFFER_SIZE, MALLOC, FREE);
        assert(con->rbuf);
        con->flag_rbuf = 1;
    }
    if (!con->rrbuf) {
        con->rrbuf = buffer_create(WSCONN_BUFFER_SIZE, MALLOC, FREE);
        assert(con->rrbuf);
        con->flag_rrbuf = 1;
    }
    if (!con->wbuf) {
        con->wbuf = buffer_create(WSCONN_BUFFER_SIZE, MALLOC, FREE);
        assert(con->wbuf);
        con->flag_wbuf = 1;
    }
    return reactor_register(con->r, &con->h, EVENT_IN);
}

// return = 0 success
// return < 0, fail maybe full
int
wsconn_send(wsconn_t* con, const char* buffer, int buflen) {
    int nwrite;
    int8_t head, sz, c;
    int16_t len;
    int64_t len64;
    if (!con || !buffer || buflen < 0) return -1;
    if (0 != wsconn_established(con)) return -1;

    nwrite = buffer_write_len(con->wbuf);
    // text mode
    head = 0x81;
    buffer_write(con->wbuf, (char*)&head, 1);
    if (buflen < 126) {
        if (nwrite < buflen + 2) return -1;
        sz = buflen;
        buffer_write(con->wbuf, (char*)&sz, 1);
        buffer_write(con->wbuf, buffer, buflen);
    } else if (buflen <= 65535) {
        if (nwrite < buflen + 4) return -1;
        c = 126;
        buffer_write(con->wbuf, (char*)&c, 1);
        len = buflen;
#if defined(OS_LITTLE_ENDIAN)
        buffer_write(con->wbuf, (char*)&len + 1, 1);
        buffer_write(con->wbuf, (char*)&len, 1);
#elif defined(OS_BIG_ENDIAN)
        buffer_write(con->wbuf, (char*)&len, 2);
#endif
        buffer_write(con->wbuf, buffer, buflen);
    } else {
        if (nwrite < buflen + 10) return -1;
        c = 127;
        buffer_write(con->wbuf, (char*)&c, 1);
        len64 = buflen;
#if defined(OS_LITTLE_ENDIAN)
        buffer_write(con->wbuf, (char*)&len64 + 7, 1);
        buffer_write(con->wbuf, (char*)&len64 + 6, 1);
        buffer_write(con->wbuf, (char*)&len64 + 5, 1);
        buffer_write(con->wbuf, (char*)&len64 + 4, 1);
        buffer_write(con->wbuf, (char*)&len64 + 3, 1);
        buffer_write(con->wbuf, (char*)&len64 + 2, 1);
        buffer_write(con->wbuf, (char*)&len64 + 1, 1);
        buffer_write(con->wbuf, (char*)&len64, 1);
#elif defined(OS_BIG_ENDIAN)
        buffer_write(con->wbuf, (char*)&len64, 8);
#endif
        buffer_write(con->wbuf, buffer, buflen);
    }

    // add out events (no buffer before send)
    reactor_modify(con->r, &con->h, (EVENT_IN | EVENT_OUT));
    return 0;
}

int
wsconn_stop(wsconn_t* con) {
    if (!con || con->h.fd == INVALID_SOCK)
        return -1;
    reactor_unregister(con->r, &con->h);
    sock_close(con->h.fd);
    con->h.fd = INVALID_SOCK;
    return 0;
}

inline int
wsconn_established(wsconn_t* con) {
    return con ? (con->status == WS_ESTABLISHED ? 0 : -1) : -1;
}

