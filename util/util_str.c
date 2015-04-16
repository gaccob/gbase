#include <ctype.h>
#include "util_str.h"

uint32_t
util_str2int(const char *key) {
    char res_decimals[15] = "";
    char *tail_res = res_decimals;
    uint8_t space_count = 0;
    uint8_t i = 0;
    do {
        if (isdigit((unsigned char)(key[i])))
            strncat(tail_res++, &key[i], 1);
        if (key[i] == ' ')
            space_count++;
    } while (key[++i]);
    return ((uint32_t) strtoul(res_decimals, NULL, 10) / space_count);
}

// dst == NULL, only return calculated escape-size
// dst != NULL, do escape and return escape-size
size_t
util_uri_escape(char* dst, const char* src, size_t size, int32_t type) {
    // blank, #, %, ?, %00-%1F, %7F-%FF
    static uint32_t uri[] = {
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
                    // ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!
        0x80000029, // 1000 0000 0000 0000  0000 0000 0010 1001
                    // _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
                    //  ~}| {zyx wvut srqp  onml kjih gfed cba`
        0x80000000, // 1000 0000 0000 0000  0000 0000 0000 0000
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff  // 1111 1111 1111 1111  1111 1111 1111 1111
    };

    // blank, #, %, &, +, ?, %00-%1F, %7F-%FF
    static uint32_t args[] = {
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
                    // ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!
        0x88000869, // 1000 1000 0000 0000  0000 1000 0110 1001
                    // _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
                    //  ~}| {zyx wvut srqp  onml kjih gfed cba`
        0x80000000, // 1000 0000 0000 0000  0000 0000 0000 0000
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff  // 1111 1111 1111 1111  1111 1111 1111 1111
    };

    // not alpha, digit, "-" "." "_" "~"
    static uint32_t component[] = {
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
                    // ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!
        0xfc009fff, // 1111 1100 0000 0000  1001 1111 1111 1111
                    // _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@
        0x78000001, // 0111 1000 0000 0000  0000 0000 0000 0001
                    //  ~}| {zyx wvut srqp  onml kjih gfed cba`
        0xb8000001, // 1011 1000 0000 0000  0000 0000 0000 0001
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff  // 1111 1111 1111 1111  1111 1111 1111 1111
    };

    // blank, #, ", %, ', %00-%1F, %7F-%FF
    static uint32_t url[] = {
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
                    // ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!
        0x000000ad, // 0000 0000 0000 0000  0000 0000 1010 1101
                    // _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
                    //  ~}| {zyx wvut srqp  onml kjih gfed cba`
        0x80000000, // 1000 0000 0000 0000  0000 0000 0000 0000
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
        0xffffffff  // 1111 1111 1111 1111  1111 1111 1111 1111
    };

    // blank, %, %00-%01F
    static uint32_t mail[] = {
        0xffffffff, // 1111 1111 1111 1111  1111 1111 1111 1111
                    // ?>=< ;:98 7654 3210  /.-, +*)( '&%$ #"!
        0x00000021, // 0000 0000 0000 0000  0000 0000 0010 0001
                    // _^]\ [ZYX WVUT SRQP  ONML KJIH GFED CBA@
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
                    //  ~}| {zyx wvut srqp  onml kjih gfed cba`
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
        0x00000000, // 0000 0000 0000 0000  0000 0000 0000 0000
    };

    static uint32_t* map[] = {
        uri, args, component, url, mail
    };

    uint32_t* escape;
    size_t n = 0;
    static unsigned char hex[] = "0123456789abcdef";
    if (type < 0 || type >= UTIL_ESCAPE_MAX) return 0;
    escape = map[type];

    // only return escaped-size
    if (!dst) {
        while (size) {
            if (escape[*src >> 5] & (1 << (*src & 0x1f))) {
                ++ n;
            }
            ++ src;
            -- size;
        }
        return n;
    }

    // do escape
    while (size) {
        if (escape[*src >> 5] & (1 << (*src & 0x1f))) {
            *dst ++ = '%';
            *dst ++ = hex[*src >> 4];
            *dst ++ = hex[*src & 0xf];
            ++ src;
            n += 3;
        } else {
            *dst ++ = *src ++;
            ++ n;
        }
        -- size;
    }
    return n;
}

void
util_uri_unescape(char** dst, char** src, size_t size) {
    char *d, *s, ch, c, decoded;
    enum {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    d = *dst;
    s = *src;
    state = 0;
    decoded = 0;
    while (size--) {
        ch = *s++;
        switch (state) {
            case sw_usual:
                if (ch == '?') {
                    *d++ = ch;
                    goto done;
                }
                if (ch == '%') {
                    state = sw_quoted;
                    break;
                }
                *d++ = ch;
                break;

            case sw_quoted:
                // 0-9
                if (ch >= '0' && ch <= '9') {
                    decoded = (char)(ch - '0');
                    state = sw_quoted_second;
                    break;
                }
                // a-f
                c = (char) (ch | 0x20);
                if (c >= 'a' && c <= 'f') {
                    decoded = (char) (c - 'a' + 10);
                    state = sw_quoted_second;
                    break;
                }
                // invalid
                state = sw_usual;
                *d++ = ch;
                break;

            case sw_quoted_second:
                // 0-9
                state = sw_usual;
                if (ch >= '0' && ch <= '9') {
                    ch = (char) ((decoded << 4) + ch - '0');
                    *d++ = ch;
                    break;
                }
                // a-f
                c = (char) (ch | 0x20);
                if (c >= 'a' && c <= 'f') {
                    ch = (char) ((decoded << 4) + c - 'a' + 10);
                    if (ch == '?') {
                        *d++ = ch;
                        goto done;
                    }
                    *d++ = ch;
                    break;
                }
                break;
        }
    }

done:
    *dst = d;
    *src = s;
}

// dst == NULL, only return calculated escape-size
// dst != NULL, do escape and return escape-size
size_t
util_html_escape(char* dst, const char* src, size_t size) {
    char ch;
    size_t len = 0;

    if (!dst) {
        while (size) {
            switch (*src++) {
                case '<':
                    // &lt;
                    len += 4;
                    break;
                case '>':
                    // &gt;
                    len += 4;
                    break;
                case '&':
                    // &amp;
                    len += 5;
                    break;
                case '"':
                    // &quot;
                    len += 6;
                    break;
                default:
                    ++ len;
                    break;
            }
            size--;
        }
        return len;
    }

    while (size) {
        ch = *src++;
        switch (ch) {
            case '<':
                *dst++ = '&';
                *dst++ = 'l';
                *dst++ = 't';
                *dst++ = ';';
                len += 4;
                break;
            case '>':
                *dst++ = '&';
                *dst++ = 'g';
                *dst++ = 't';
                *dst++ = ';';
                len += 4;
                break;
            case '&':
                *dst++ = '&';
                *dst++ = 'a';
                *dst++ = 'm';
                *dst++ = 'p';
                *dst++ = ';';
                len += 5;
                break;
            case '"':
                *dst++ = '&';
                *dst++ = 'q';
                *dst++ = 'u';
                *dst++ = 'o';
                *dst++ = 't';
                *dst++ = ';';
                len += 6;
                break;
            default:
                *dst++ = ch;
                ++ len;
                break;
        }
        size--;
    }
    return len;
}

// dump to heximal
void
util_hex_dump(char* dst, char* src, size_t s) {
    static char hex[] = "0123456789ABCDEF";
    while (s --) {
        *dst++ = hex[*src >> 4];
        *dst++ = hex[*src & 0xf];
    }
}

