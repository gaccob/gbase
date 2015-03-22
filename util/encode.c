#include "encode.h"

static int
_get_unicode(const char* str, int n) {
    int i;
    int unicode = str[0] & ((1 << (8-n)) - 1);
    for (i=1; i<n; i++) {
        unicode = unicode << 6 | ((uint8_t)str[i] & 0x3f);
    }
    return unicode;
}

int
utf8_unicode(char** utf8, int* unicode) {
    uint8_t c;
    if (!utf8 || !(*utf8) || !unicode) {
        return -1;
    }
    if (**utf8 == 0) {
        return -1;
    }
    c = (*utf8)[0];
    if ((c & 0x80) == 0) {
        *unicode = _get_unicode(*utf8, 1);
        *utf8 += 1;
    } else if ((c & 0xe0) == 0xc0) {
        *unicode = _get_unicode(*utf8, 2);
        *utf8 += 2;
    } else if ((c & 0xf0) == 0xe0) {
        *unicode = _get_unicode(*utf8, 3);
        *utf8 += 3;
    } else if ((c & 0xf8) == 0xf0) {
        *unicode = _get_unicode(*utf8, 4);
        *utf8 += 4;
    } else if ((c & 0xfc) == 0xf8) {
        *unicode = _get_unicode(*utf8, 5);
        *utf8 += 5;
    } else if ((c & 0xfe) == 0xfc) {
        *unicode = _get_unicode(*utf8, 6);
        *utf8 += 6;
    } else {
        return -1;
    }
    return 0;
}


