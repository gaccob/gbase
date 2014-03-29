#include "connbuffer.h"

typedef struct connbuffer_t {
    char* buffer;
    int buffer_size;
    int read_pos;
    int write_pos;
    int drift_threshold;
    connbuffer_malloc malloc_func;
    connbuffer_free free_func;
} connbuffer_t;

// buffer_size hint: 4 * max pkg size
connbuffer_t*
connbuffer_create(int buffer_size, connbuffer_malloc malloc_func,
                  connbuffer_free free_func) {
    connbuffer_t* cb;
    if (buffer_size <= 0) {
        return NULL;
    }
    cb = (connbuffer_t*)MALLOC(sizeof(connbuffer_t));
    assert(cb);
    cb->buffer_size = buffer_size;
    cb->read_pos = 0;
    cb->write_pos = 0;
    cb->drift_threshold = buffer_size / 2;
    cb->malloc_func = malloc_func ? malloc_func : MALLOC;
    cb->free_func = free_func ? free_func : FREE;
    cb->buffer = (char*)cb->malloc_func(buffer_size);
    assert(cb->buffer);
    return cb;
}

int
connbuffer_release(connbuffer_t* cb) {
    if (cb) {
        cb->free_func(cb->buffer);
        FREE(cb);
    }
    return 0;
}

// read data in buffer to dest
// return: read bytes
int
connbuffer_read(connbuffer_t* cb, char* dest, int len) {
    if (!cb|| !dest || len < 0) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (len > connbuffer_read_len(cb)) {
        return connbuffer_read(cb, dest, connbuffer_read_len(cb));
    }
    memcpy(dest, connbuffer_read_buffer(cb), len);
    return connbuffer_read_nocopy(cb, len);
}

int
connbuffer_read_nocopy(connbuffer_t* cb, int len) {
    if (!cb || len < 0) {
        return -1;
    }
    if (len > connbuffer_read_len(cb)) {
        return connbuffer_read_nocopy(cb, connbuffer_read_len(cb));
    }
    cb->read_pos += len;
    assert(cb->write_pos >= cb->read_pos);
    // check drift threshold
    if (cb->read_pos > cb->drift_threshold) {
        const char* shift = connbuffer_read_buffer(cb);
        // threshold is half size, so memcpy src & dst will never overlap to escape memmove
        memcpy(cb->buffer, shift, cb->write_pos - cb->read_pos);
        cb->write_pos -= cb->read_pos;
        cb->read_pos = 0;
    }
    return len;
}

// only read and not pick out
// return: read bytes
int
connbuffer_peek(connbuffer_t* cb, char* dest, int len) {
    if (!cb || len < 0 || !dest) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (len > connbuffer_read_len(cb)) {
        return connbuffer_peek(cb, dest, connbuffer_read_len(cb));
    }
    memcpy(dest, connbuffer_read_buffer(cb), len);
    return len;
}

// write data from src to connbuffer
// return: write bytes
int
connbuffer_write(connbuffer_t* cb, const char* src, int len) {
    if (!cb || !src || len < 0) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (connbuffer_write_len(cb) < len) {
        return connbuffer_write(cb, src, connbuffer_write_len(cb));
    }
    memcpy(connbuffer_write_buffer(cb), src, len);
    return connbuffer_write_nocopy(cb, len);
}

int
connbuffer_write_nocopy(connbuffer_t* cb, int len) {
    if (connbuffer_write_len(cb) < len) {
        return connbuffer_write_nocopy(cb, connbuffer_write_len(cb));
    }
    if (len < 0) {
        return -1;
    }
    cb->write_pos += len;
    assert(cb->write_pos <= cb->buffer_size);
    return len;
}

int
connbuffer_reset(connbuffer_t* cb) {
    if (!cb) {
        return -1;
    }
    cb->read_pos = cb->write_pos = 0;
    return 0;
}

const char*
connbuffer_debug(connbuffer_t* cb) {
    static char debug_str[64];
    if (!cb) {
        return NULL;
    }
    memset(debug_str, 0, sizeof(debug_str));
    snprintf(debug_str, sizeof(debug_str),
        "size=%d, read_pos=%d, write_pos=%d",
        cb->buffer_size,
        cb->read_pos,
        cb->write_pos);
    return debug_str;
}

char*
connbuffer_read_buffer(connbuffer_t* cb) {
    return cb->buffer + cb->read_pos;
}

char*
connbuffer_write_buffer(connbuffer_t* cb) {
    return cb->buffer + cb->write_pos;
}

int
connbuffer_read_len(connbuffer_t* cb) {
    if (!cb) {
        return -1;
    }
    return cb->write_pos > cb->read_pos
        ?  (cb->write_pos - cb->read_pos) : 0;
}

int
connbuffer_write_len(connbuffer_t* cb) {
    if (!cb) {
        return -1;
    }
    return cb->buffer_size - cb->write_pos;
}

