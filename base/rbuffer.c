#include <assert.h>

#include "core/atom.h"
#include "rbuffer.h"

struct rbuffer_t {
    uint32_t size;
    volatile atom_t read_pos;
    volatile atom_t write_pos;
    char buffer[0];
};

rbuffer_t*
rbuffer_create(uint32_t size) {
	rbuffer_t* r;
    // round up by 2^n
    if (size & (size - 1)) {
        size = ROUNDUP(size);
    }
	// malloc
    r = (rbuffer_t*)MALLOC(sizeof(rbuffer_t) + size);
    if (!r) { return NULL; }
    r->size = size;
    atom_set(&r->read_pos, 0);
    atom_set(&r->write_pos, 0);
    return r;
}

rbuffer_t*
rbuffer_attach(void* mem, size_t mem_size) {
    rbuffer_t* r;
    uint32_t size;
    if (!mem || mem_size < sizeof(struct rbuffer_t)) {
        return NULL;
    }
    size = mem_size - sizeof(struct rbuffer_t);
    if (size & (size - 1)) {
        size = ROUNDDOWN(size);
    }
    if (size == 0) {
        return NULL;
    }
    // assignment
    r = (rbuffer_t*)mem;
    r->size = size;
    atom_set(&r->read_pos, 0);
    atom_set(&r->write_pos, 0);
    return r;
}

void
rbuffer_release(rbuffer_t* r) {
    if (r) { FREE(r); }
}

size_t
rbuffer_size(rbuffer_t* r) {
    return r ? r->size : 0;
}

size_t
rbuffer_head_size() {
    return sizeof(rbuffer_t);
}

uint32_t
rbuffer_read_bytes(rbuffer_t* r) {
    uint32_t write_pos = r->write_pos;
    uint32_t read_pos = r->read_pos;
    uint32_t min_len = sizeof(uint32_t);
    if (write_pos - read_pos >= min_len) {
        return write_pos - read_pos - min_len;
    }
    return 0;
}

uint32_t
rbuffer_write_bytes(rbuffer_t* r) {
    uint32_t write_pos = r->write_pos;
    uint32_t read_pos = r->read_pos;
    uint32_t min_len = sizeof(uint32_t);
    if (write_pos - read_pos >= r->size - min_len) {
        return 0;
    }
    return r->size - min_len - (write_pos - read_pos);
}

int
rbuffer_read(rbuffer_t* r, char* buf, size_t* buf_size) {
    int ret = rbuffer_peek(r, buf, buf_size);
    if (0 == ret) {
        atom_add(&r->read_pos, (sizeof(uint32_t) + *buf_size));
    }
    return ret;
}

int
rbuffer_peek(rbuffer_t* r, char* buf, size_t* buf_size) {
    uint32_t read_to_tail, least, read_to_tail_least, read_len, read_to_tail_bytes;
    if (!r || !buf || !buf_size) {
        return -1;
    }
    if (0 == rbuffer_read_bytes(r)) {
        return -1;
    }

    read_to_tail = r->size - (r->read_pos & (r->size - 1));
    least = sizeof(uint32_t);
    read_to_tail_least = (read_to_tail < least ? read_to_tail : least);

    // get data length
    memcpy(&read_len, r->buffer + (r->read_pos & (r->size - 1)), read_to_tail_least);
    if (read_to_tail_least < least) {
        memcpy((char*)&read_len + read_to_tail_least, r->buffer, least - read_to_tail_least);
    }
    // check data length
    if (read_len > r->size - least || read_len > *buf_size) {
        return -1;
    }

    // copy data from buffer
    if (read_to_tail_least < least) {
        memcpy(buf, r->buffer + least - read_to_tail_least, read_len);
    } else {
        read_to_tail -= least;
        read_to_tail_bytes = (read_len < read_to_tail ? read_len : read_to_tail);
        memcpy(buf, r->buffer + ((r->read_pos + least) & (r->size - 1)), read_to_tail_bytes);
        if (read_to_tail_bytes < read_len) {
            memcpy(buf + read_to_tail_bytes, r->buffer, read_len - read_to_tail_bytes);
        }
    }

    // set read buf size
    *buf_size = read_len;
    return 0;
}

int
rbuffer_write(rbuffer_t* r, const char* buf, size_t buf_size) {
    uint32_t len_size, nwrites, write_to_tail, min_len_size, write_to_tail_bytes;
    if (!r || !buf) return -1;
    if (0 == buf_size) return 0;

    len_size = sizeof(uint32_t);
    nwrites = buf_size + len_size;
    if (rbuffer_write_bytes(r) < buf_size) {
        return -1;
    }
    write_to_tail = r->size - (r->write_pos & (r->size - 1));
    min_len_size = (len_size < write_to_tail ? len_size : write_to_tail);

    // write length first
    memcpy(r->buffer + (r->write_pos & (r->size - 1)), &buf_size, min_len_size);
    if (min_len_size < len_size) {
        memcpy(r->buffer, (char*)&buf_size + min_len_size, len_size - min_len_size);
    }

    // write data to buffer
    if (write_to_tail > len_size) {
        write_to_tail -= len_size;
        write_to_tail_bytes = (buf_size < write_to_tail ? buf_size : write_to_tail);
        memcpy(r->buffer + ((r->write_pos + len_size) & (r->size - 1)), buf, write_to_tail_bytes);
        if (write_to_tail_bytes < buf_size) {
            memcpy(r->buffer, buf + write_to_tail_bytes, buf_size - write_to_tail_bytes);
        }
    } else {
        memcpy(r->buffer + len_size - min_len_size, buf, buf_size);
    }

    // set write pos
    atom_add(&r->write_pos, nwrites);
    return 0;
}

