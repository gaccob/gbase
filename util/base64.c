#include <ctype.h>
#include "base64.h"

const char _base64_table[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

#define _BASE64_ENCODE(a4, b3) \
    a4[0] = (b3[0] & 0xfc) >> 2; \
    a4[1] = ((b3[0] & 0x03) << 4) | ((b3[1] & 0xf0) >> 4); \
    a4[2] = ((b3[1] & 0x0f) << 2) | ((b3[2] & 0xc0) >> 6); \
    a4[3] = b3[2] & 0x3f;

int32_t base64_encode(char* dst, const char* src, size_t sz)
{
    size_t idx = 0, i = 0, j;
    uint8_t a[4], b[3];
    if (!dst || !src || sz == 0) return -1;

    while (sz-- > 0 && src[idx] != 0) {
        b[i++] = src[idx++];
        if (i == 3) {
            _BASE64_ENCODE(a, b);
            for (j = 0; j < 4; j ++) {
                *dst++ = _base64_table[a[j]];
            }
            i = 0;
        }
    }
    if (i > 0) {
        for (j = i; j < 3; j++) {
            b[j] = '\0';
        }
        _BASE64_ENCODE(a, b);
        for (j = 0; j < i + 1; j++) {
            *dst++ = _base64_table[a[j]];
        }
        while (i++ < 3) {
            *dst++ = '=';
        }
    }
    *dst++ = 0;
    return 0;
}

#define _BASE64_C(c) (isalnum(c) || ((c) == '+') || ((c) == '/'))
#define _BASE64_DECODE(a3, b4) \
    a3[0] = (b4[0] << 2) + ((b4[1] & 0x30) >> 4); \
    a3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2); \
    a3[2] = ((b4[2] & 0x03) << 6) + b4[3];

int32_t base64_decode(char* dst, const char* src, size_t sz)
{
	size_t i = 0, idx = 0, j;
    uint8_t a[3], b[4];
	if (!dst || !src || sz == 0) return -1;

    while (sz-- > 0 && src[idx] != '=') {
        if (!_BASE64_C(src[idx])) return -1;
        b[i++] = src[idx++];
        if (i == 4) {
            for (j = 0; j < i; j++) {
                b[j] = strchr(_base64_table, b[j]) - _base64_table;
            }
            _BASE64_DECODE(a, b);
            for (j = 0; j < 3; j++) {
                *dst++ = a[j];
            }
            i = 0;
        }
    }
    if (i > 0) {
        for (j = i; j < 4; j++) {
            b[j] = 0;
        }
        for (j = 0; j < 4; j++) {
            b[j] = strchr(_base64_table, b[j]) - _base64_table;
        }
        _BASE64_DECODE(a, b);
        for (j = 0; j < i - 1; j++) {
            *dst++ = a[j];
        }
    }
    *dst++ = 0;
    return 0;
}
