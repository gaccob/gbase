#ifndef WEBSOCKET_H_
#define WEBSOCKET_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "os/os_def.h"
#include "net/connector.h"
#include "net/reactor.h"

typedef void (*ws_handshake_cb)(int fd);

enum {
    WS_INIT,
    WS_ESTABLISHED,
    WS_FINI,
};

struct websock_t
{
    struct connector_t* con;
    ws_handshake_cb hs_cb;
    int8_t status;
};

struct websock_t* ws_init(struct connector_t* con,
                          ws_handshake_cb hs_cb)
{
    if (!con) return NULL;
    struct websock_t* ws = (struct websocket_t*)MALLOC(sizeof(struct websocket_t));
    ws->con = con;
    ws->hs_cb = hs_cb;
    ws->status = WS_INIT;
    return ws;
}

void ws_release();

int32_t ws_established(websocket_t* ws)
{
    if (!ws) return -1;
    return ws->status == WS_ESTABLISHED ? 0 : -1;
}

int32_t _ws_read(struct websocket_t* ws)
{
    char* buffer = connbuffer_read_buffer(ws->con->read_buf);
    int32_t nread = connbuffer_read_len(ws->con->read_buf);
    int32_t from = 0;
    int32_t ret = -1;
    while (from < nread - 4)
    {
        if (buffer[from] == '\r'
            && buffer[from + 1] == '\n'
            && buffer[from + 2] == '\r'
            && buffer[from + 3] == '\n')
        {
            int32_t ret = -1;
            if (0 == ws_established(ws))
                ret = _ws_handshake(buffer, from + 4);
            else
                ret = _ws_frame(buffer, from + 4);
            if (0 == ret)
                connbuffer_read_nocopy(ws->con->read_buf, from + 4);
            else
                return ret;
            from = 0;
            buffer = connbuffer_read_buffer(ws->con->read_buf);
            nread = connbuffer_read_len(ws->con->read_buf);
        }
        from ++;
    }
    return 0;
}

int32_t _ws_fetch_line(const char* src, int32_t* sz, char* field, char* value)
{
    if (*sz < 2) return -1;
    int32_t lowercase = 1;
    int32_t is_field = 1;
    int32_t size = *sz;
    int32_t i = 0, j = 0;
    field[0] = 0;
    for (; i < size - 1; i++)
    {
        if (src[i] == '\r' && src[i+1] == '\n')
        {
            value[j] = 0;
            *sz = i + 2;
            return 0;
        }
        if (is_field)
        {
            if (src[i] == ':')
            {
                field[j] = 0;
                is_field = 0;
                j = 0;
            }
            else if (lowercase && src[i] >= 'A' && src[i] <= 'Z')
            {
                field[j++] = src[i] + 'a' - 'A';
            }
            else
            {
                field[j++] = src[i];
            }
        }
        else
        {
            value[j++] = src[i];
        }
    }
    return -1;
}

int32_t _ws_trim_lowcase_strcmp(const char* s1, const char* s2)
{
    int32_t i=0, j=0;
    while (s1[i] && s2[j])
    {
        if (s1[i] == ' ')
        {
            i++;
            continue;
        }
        if (s2[j] == ' ')
        {
            j++;
            continue;
        }
        if (s1[i] != )
    }
}

/*
1. websocket handshake for flash
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


2. websocket handshake for version > 13: sha1 + base64 encrypt
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


3. websocket handshake for no version: md5 encrypt
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

#define WS_FLASH (WS_FLAG_UPGRAGE | WS_FLAG_CONNECTION | WS_FLAG_HOST | WS_FLAG_ORIGIN)
#define WS_SHA (WS_FLAG_UPGRAGE | WS_FLAG_CONNECTION | WS_FLAG_HOST | WS_FLAG_SEC_ORIGIN \
                WS_FLAG_SEC_KEY | WS_FLAG_SEC_VERSION)
#define WS_MD5 (WS_FLAG_HOST | WS_FLAG_CONNECTION | WS_FLAG_SEC_KEY1 | WS_FLAG_SEC_KEY2 \
                WS_FLAG_UPGRAGE | WS_FLAG_ORIGIN | WS_FLAG_SEC_PROTOCOL)

const char* const WS_HEAD_GET = "get";
const char* const WS_HEAD_HOST = "host";
const char* const WS_HEAD_CONNECTION = "connection";
const char* const WS_HEAD_UPGRADE = "upgrade";
const char* const WS_HEAD_ORIGIN = "origin";
const char* const WS_HEAD_SEC_ORIGIN = "sec-websocket-origin";
const char* const WS_HEAD_SEC_KEY = "sec-websocket-key";
const char* const WS_HEAD_SEC_VERSION = "sec-websocket-version";
const char* const WS_HEAD_SEC_KEY1 = "sec-websocket-key1";
const char* const WS_HEAD_SEC_KEY2 = "sec-websocket-key2";
const char* const WS_HEAD_WS_ORIGIN = "Websocket-Origin";
const char* const WS_HEAD_WS_LOCATION = "websocket-location";
const char* const WS_HEAD_SEC_ACCEPT = "sec-websocket-accept";
const char* const WS_HEAD_SEC_LOCATION = "sec-websocket-location";
const char* const WS_HEAD_SEC_PROTOCOL = "sec-websocket-protocol"

int32_t _ws_handshake(const char* frame, int32_t len)
{
    if(!frame || len <= 0)
        return -1;
    int32_t flag = WS_FLAG_NULL;
    char field[32], value[128];
    const char* src = frame;
    int32_t sz = len;
    while (_ws_fetch_line(frame, &sz, field, value) == 0)
    {
        printf("%s:%s\n", field, value);
        if(strcmp(field, WS_HEAD_UPGRADE))
        {
            flag |= WS_FLAG_UPGRAGE;
        }
        else if(strcmp(field, WS_HEAD_CONNECTION))
        {
            flag |= WS_FLAG_CONNECTION;
        }
        else if(strcmp(field, WS_HEAD_HOST))
        {
            flag |= WS_FLAG_HOST;
        }
        else if(strcmp(field, WS_HEAD_ORIGIN))
        {
            flag |= WS_FLAG_ORIGIN;
        }
        else if(strcmp(field, WS_HEAD_SEC_ORIGIN))
        {
            flag |= WS_FLAG_SEC_ORIGIN;
        }
        else if(strcmp(field, WS_HEAD_SEC_KEY))
        {
            flag |= WS_FLAG_SEC_KEY;
        }
        else if(strcmp(field, WS_HEAD_SEC_VERSION))
        {
            flag |= WS_FLAG_SEC_VERSION;
        }
        else if(strcmp(field, WS_HEAD_SEC_KEY1))
        {
            flag |= WS_FLAG_SEC_KEY1;
        }
        else if(strcmp(field, WS_HEAD_SEC_KEY2))
        {
            flag |= WS_FLAG_SEC_KEY2;
        }
        else if(strcmp(field, WS_HEAD_SEC_PROTOCOL))
        {
            flag |= WS_FLAG_SEC_PROTOCOL;
        }
    }

    if (flag & WS_FLASH)
        goto WS_FLASH_STEP;

    if (flag & WS_SHA)
        goto WS_SHA_STEP;

    if (flag & WS_MD5)
        goto WS_MD5_STEP;

    printf("invalid flag=%d\n", flag);
    return -1;

WS_FLASH_STEP:

WS_SHA_STEP:

WS_MD5_STEP:
    // TODO: callback handshake
}

int32_t _ws_frame(const char* frame, int len)
{
    // TODO: callback connector-read
}

/*
bool WebSocket::doHandshake() {
    char temp[128];
    char key[80];
    char bite;

    bool hasUpgrade = false;
    bool hasConnection = false;
    bool isSupportedVersion = false;
    bool hasHost = false;
    bool hasOrigin = false;
    bool hasKey = false;

    byte counter = 0;
    while ((bite = client.read()) != -1) {
        temp[counter++] = bite;

        if (bite == '\n' || counter > 127) { // EOL got, or too long header. temp should now contain a header string
            temp[counter - 2] = 0; // Terminate string before CRLF

            #ifdef DEBUG
                Serial.print("Got header: ");
                Serial.println(temp);
            #endif

            // Ignore case when comparing and allow 0-n whitespace after ':'. See the spec:
            // http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html
            if (!hasUpgrade && strstr(temp, "Upgrade:")) {
                // OK, it's a websockets handshake for sure
                hasUpgrade = true;
            } else if (!hasConnection && strstr(temp, "Connection: ")) {
                hasConnection = true;
            } else if (!hasOrigin && strstr(temp, "Origin:")) {
                hasOrigin = true;
            } else if (!hasHost && strstr(temp, "Host: ")) {
                hasHost = true;
            } else if (!hasKey && strstr(temp, "Sec-WebSocket-Key: ")) {
                hasKey = true;
                strtok(temp, " ");
                strcpy(key, strtok(NULL, " "));
            } else if (!isSupportedVersion && strstr(temp, "Sec-WebSocket-Version: ") && strstr(temp, "13")) {
                isSupportedVersion = true;
            }

            counter = 0; // Start saving new header string
        }
    }

    // Assert that we have all headers that are needed. If so, go ahead and
    // send response headers.
    if (hasUpgrade && hasConnection && isSupportedVersion && hasHost && hasOrigin && hasKey) {
        strcat(key, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"); // Add the omni-valid GUID
        Sha1.init();
        Sha1.print(key);
        uint8_t *hash = Sha1.result();
        base64_encode(temp, (char*)hash, 20);
        client.print("HTTP/1.1 101 Switching Protocols\r\n");
        client.print("Upgrade: websocket\r\n");
        client.print("Connection: Upgrade\r\n");
        client.print("Sec-WebSocket-Accept: ");
        client.print(temp);
        client.print(CRLF);
        client.print(CRLF);
    } else {
        // Nope, failed handshake. Disconnect
        return false;
    }

    return true;
}
*/


#ifdef __cplusplus
}
#endif

#endif // WEBSOCKET_H_
