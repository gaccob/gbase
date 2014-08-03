#include "serialize.h"

struct serial_t {
    char* buffer;
    size_t size;
    size_t cur;
};

serial_t*
serial_create(char* buffer, size_t size) {
    serial_t* s = (serial_t*)MALLOC(sizeof(serial_t));
    s->buffer = buffer;
    s->size = size;
    s->cur = 0;
    return s;
}

void
serial_release(serial_t* s) {
    if(s) FREE(s);
}

int
serial_write8(serial_t* s, uint8_t src) {
    if(s->size - s->cur < sizeof(src)) {
        return -1;
    }
    s->buffer[s->cur ++] = src;
    return 0;
}
int
serial_read8(serial_t* s, uint8_t* dest) {
    if(s->size - s->cur < sizeof(*dest)) {
        return -1;
    }
    *dest = s->buffer[s->cur ++];
    return 0;
}

int
serial_write16(serial_t* s, uint16_t src) {
    if(s->size - s->cur < sizeof(src)) {
        return -1;
    }
#if defined(OS_LITTLE_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)(src & 0xFF00);
    s->buffer[s->cur ++] = (uint8_t)((src & 0xFF) >> 8);
#elif defined(OS_BIG_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)(src & 0xFF);
    s->buffer[s->cur ++] = (uint8_t)((src & 0xFF00) >> 8);
#endif
    return 0;
}
int
serial_read16(serial_t* s, uint16_t* dest) {
    uint16_t b1, b2;
    if(s->size - s->cur < sizeof(*dest)) {
        return -1;
    }
    b1 = (uint8_t)s->buffer[s->cur ++];
    b2 = (uint8_t)s->buffer[s->cur ++];
#if defined(OS_LITTLE_ENDIAN)
    *dest = (b1 << 8 | b2);
#elif defined(OS_BIG_ENDIAN)
    *dest = (b1 | b2 << 8);
#endif
    return 0;
}

const uint64_t _serail_32u24 = ((uint32_t)0xFF << 24);
const uint64_t _serail_32u16 = ((uint32_t)0xFF << 16);
const uint64_t _serail_32u8 = ((uint32_t)0xFF << 8);
const uint64_t _serail_32u0 = (uint32_t)0xFF;
int
serial_write32(serial_t* s, uint32_t src) {
    if(s->size - s->cur < sizeof(src)) {
        return -1;
    }
#if defined(OS_LITTLE_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u24) >> 24);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u16) >> 16);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u8) >> 8);
    s->buffer[s->cur ++] = (uint8_t)(src & _serail_32u0);
#elif defined(OS_BIG_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)(src & _serail_32u0);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u8) >> 8);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u16) >> 16);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_32u24) >> 24);
#endif
    return 0;
}
int
serial_read32(serial_t* s, uint32_t* dest) {
    uint32_t b1, b2, b3, b4;
    if(s->size - s->cur < sizeof(*dest)) {
        return -1;
    }
    b1 = (uint8_t)s->buffer[s->cur ++];
    b2 = (uint8_t)s->buffer[s->cur ++];
    b3 = (uint8_t)s->buffer[s->cur ++];
    b4 = (uint8_t)s->buffer[s->cur ++];
#if defined(OS_LITTLE_ENDIAN)
    *dest = (b1 << 24 | b2 << 16 | b3 << 8 | b4);
#elif defined(OS_BIG_ENDIAN)
    *dest = (b1 | b2 << 8 | b3 << 16 | b4 << 24);
#endif
    return 0;
}

const uint64_t _serail_64u56 = ((uint64_t)0xFF << 56);
const uint64_t _serail_64u48 = ((uint64_t)0xFF << 48);
const uint64_t _serail_64u40 = ((uint64_t)0xFF << 40);
const uint64_t _serail_64u32 = ((uint64_t)0xFF << 32);
const uint64_t _serail_64u24 = ((uint64_t)0xFF << 24);
const uint64_t _serail_64u16 = ((uint64_t)0xFF << 16);
const uint64_t _serail_64u8 = ((uint64_t)0xFF << 8);
const uint64_t _serail_64u0 = (uint64_t)0xFF;
int
serial_write64(serial_t* s, uint64_t src) {
    if(s->size - s->cur < sizeof(src)) {
        return -1;
    }
#if defined(OS_LITTLE_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u56) >> 56);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u48) >> 48);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u40) >> 40);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u32) >> 32);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u24) >> 24);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u16) >> 16);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u8) >> 8);
    s->buffer[s->cur ++] = (uint8_t)(src & _serail_64u0);
#elif defined(OS_BIG_ENDIAN)
    s->buffer[s->cur ++] = (uint8_t)(src & _serail_64u0);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u8) >> 8);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u16) >> 16);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u24) >> 24);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u32) >> 32);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u40) >> 40);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u48) >> 48);
    s->buffer[s->cur ++] = (uint8_t)((src & _serail_64u56) >> 56);
#endif
    return 0;
}
int
serial_read64(serial_t* s, uint64_t* dest) {
    uint64_t b1, b2, b3, b4, b5, b6, b7, b8;
    if(s->size - s->cur < sizeof(*dest)) {
        return -1;
    }
    b1 = (uint8_t)s->buffer[s->cur ++];
    b2 = (uint8_t)s->buffer[s->cur ++];
    b3 = (uint8_t)s->buffer[s->cur ++];
    b4 = (uint8_t)s->buffer[s->cur ++];
    b5 = (uint8_t)s->buffer[s->cur ++];
    b6 = (uint8_t)s->buffer[s->cur ++];
    b7 = (uint8_t)s->buffer[s->cur ++];
    b8 = (uint8_t)s->buffer[s->cur ++];
#if defined(OS_LITTLE_ENDIAN)
    *dest = (b1 << 56 | b2 << 48 | b3 << 40 | b4 << 32 | b5 << 24 | b6 << 16 | b7 << 8 | b8);
#elif defined(OS_BIG_ENDIAN)
    *dest = (b1 | b2 << 8 | b3 << 16 | b4 << 24 | b5 << 32 | b6 << 40 | b7 << 48 | b8 << 56);
#endif
    return 0;
}

int
serial_writef(serial_t* s, float src) {
    union{ uint32_t n; float f; } u;
    u.f = src;
    return serial_write32(s, u.n);
}
int
serial_readf(serial_t* s, float* dest) {
    int res;
    union{ uint32_t n; float f; } u;
    res = serial_read32(s, &u.n);
    if(0 == res) {
        *dest = u.f;
    }
    return res;
}

int
serial_writed(serial_t* s, double src) {
    union{ uint64_t n; double d; } u;
    u.d = src;
    return serial_write64(s, u.n);
}
int
serial_readd(serial_t* s, double* dest) {
    int res;
    union{ uint64_t n; double d; } u;
    res = serial_read64(s, &u.n);
    if(0 == res) {
        *dest = u.d;
    }
    return res;
}

int
serial_writen(serial_t* s, const char* data, uint32_t len) {
    if(s->size - s->cur < sizeof(uint32_t) + len) {
        return -1;
    }
    serial_write32(s, len);
    memcpy(s->buffer, data, len);
    s->cur += len;
    return 0;
}
int
serial_readn(serial_t* s, char* data, uint32_t* len) {
    uint32_t read_len;
    uint32_t b1, b2, b3, b4;
    if(s->size - s->cur < sizeof(uint32_t)) {
        return -1;
    }
    b1 = (uint8_t)s->buffer[s->cur];
    b2 = (uint8_t)s->buffer[s->cur + 1];
    b3 = (uint8_t)s->buffer[s->cur + 2];
    b4 = (uint8_t)s->buffer[s->cur + 3];
#if defined(OS_LITTLE_ENDIAN)
    read_len = (b1 << 24 | b2 << 16 | b3 << 8 | b4);
#elif defined(OS_BIG_ENDIAN)
    read_len = (b1 | b2 << 8 | b3 << 16 | b4 << 24);
#endif
    if(s->size - s->cur < sizeof(uint32_t) + read_len) {
        return -1;
    }
    *len = read_len;
    memcpy(data, s->buffer + 4, read_len);
    s->cur += (4 + read_len);
    return 0;
}

