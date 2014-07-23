#include "buffer.h"

typedef struct buffer_t {
    char* buffer;
    int buffer_size;
    int read_pos;
    int write_pos;
    int drift_threshold;
    buffer_malloc_func malloc_func;
    buffer_free_func free_func;
} buffer_t;

// buffer_size hint: 4 * max pkg size
buffer_t*
buffer_create(int buffer_size,
                  buffer_malloc_func malloc_func,
                  buffer_free_func free_func) {
    if (buffer_size <= 0) {
        return NULL;
    }
    buffer_t* cb = (buffer_t*)MALLOC(sizeof(buffer_t));
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
buffer_release(buffer_t* cb) {
    if (cb) {
        cb->free_func(cb->buffer);
        FREE(cb);
    }
    return 0;
}

// read data in buffer to dest
// return: read bytes
int
buffer_read(buffer_t* cb, char* dest, int len) {
    if (!cb|| !dest || len < 0) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (len > buffer_read_len(cb)) {
        return buffer_read(cb, dest, buffer_read_len(cb));
    }
    memcpy(dest, buffer_read_buffer(cb), len);
    return buffer_read_nocopy(cb, len);
}

int
buffer_read_nocopy(buffer_t* cb, int len) {
    if (!cb || len < 0) {
        return -1;
    }
    if (len > buffer_read_len(cb)) {
        return buffer_read_nocopy(cb, buffer_read_len(cb));
    }
    cb->read_pos += len;
    assert(cb->write_pos >= cb->read_pos);
    // check drift threshold
    if (cb->read_pos > cb->drift_threshold) {
        const char* shift = buffer_read_buffer(cb);
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
buffer_peek(buffer_t* cb, char* dest, int len) {
    if (!cb || len < 0 || !dest) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (len > buffer_read_len(cb)) {
        return buffer_peek(cb, dest, buffer_read_len(cb));
    }
    memcpy(dest, buffer_read_buffer(cb), len);
    return len;
}

// write data from src to buffer
// return: write bytes
int
buffer_write(buffer_t* cb, const char* src, int len) {
    if (!cb || !src || len < 0) {
        return -1;
    }
    if (0 == len) {
        return 0;
    }
    if (buffer_write_len(cb) < len) {
        return buffer_write(cb, src, buffer_write_len(cb));
    }
    memcpy(buffer_write_buffer(cb), src, len);
    return buffer_write_nocopy(cb, len);
}

int
buffer_write_nocopy(buffer_t* cb, int len) {
    if (buffer_write_len(cb) < len) {
        return buffer_write_nocopy(cb, buffer_write_len(cb));
    }
    if (len < 0) {
        return -1;
    }
    cb->write_pos += len;
    assert(cb->write_pos <= cb->buffer_size);
    return len;
}

int
buffer_reset(buffer_t* cb) {
    if (!cb) {
        return -1;
    }
    cb->read_pos = cb->write_pos = 0;
    return 0;
}

const char*
buffer_debug(buffer_t* cb) {
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
buffer_read_buffer(buffer_t* cb) {
    return cb->buffer + cb->read_pos;
}

char*
buffer_write_buffer(buffer_t* cb) {
    return cb->buffer + cb->write_pos;
}

int
buffer_read_len(buffer_t* cb) {
    if (!cb) {
        return -1;
    }
    return cb->write_pos > cb->read_pos ? (cb->write_pos - cb->read_pos) : 0;
}

int
buffer_write_len(buffer_t* cb) {
    if (!cb) {
        return -1;
    }
    return cb->buffer_size - cb->write_pos;
}

