#include "ds/connbuffer.h"

typedef struct connbuffer_t
{
    char* buffer;
    int buffer_size;
    int read_pos;
    int write_pos;
    int drift_threshold;
    connbuffer_malloc buffer_malloc;
    connbuffer_free buffer_free;
}connbuffer_t;

/* buffer_size hint: 4 * max pkg size */
struct connbuffer_t* connbuffer_init(int buffer_size, connbuffer_malloc buffer_malloc, connbuffer_free buffer_free)
{
    struct connbuffer_t* connbuffer;

    if(buffer_size <= 0)
        return NULL;

    connbuffer = (struct connbuffer_t*)MALLOC(sizeof(struct connbuffer_t));
    assert(connbuffer);
    connbuffer->buffer_size = buffer_size;
    connbuffer->read_pos = 0;
    connbuffer->write_pos = 0;
    connbuffer->drift_threshold = buffer_size / 2;
    if(buffer_malloc)
        connbuffer->buffer_malloc = buffer_malloc;
    else
        connbuffer->buffer_malloc = MALLOC;
    if(buffer_free)
        connbuffer->buffer_free = buffer_free;
    else
        connbuffer->buffer_free = FREE;
    connbuffer->buffer = (char*)connbuffer->buffer_malloc(buffer_size);
    assert(connbuffer->buffer);

    return connbuffer;
}

int connbuffer_release(struct connbuffer_t* connbuffer)
{
    if(connbuffer)
    {
        connbuffer->buffer_free(connbuffer->buffer);
        FREE(connbuffer);
    }
    return 0;
}

/*
* read data in buffer to dest
* return: read bytes
*/
int connbuffer_read(struct connbuffer_t* connbuffer, char* dest, int len)
{
    if(!connbuffer || !dest || len < 0)
        return -1;
    if(0 == len)
        return 0;

    if(len > connbuffer_read_len(connbuffer))
        return connbuffer_read(connbuffer, dest, connbuffer_read_len(connbuffer));

    memcpy(dest, connbuffer_read_buffer(connbuffer), len);
    return connbuffer_read_nocopy(connbuffer, len);
}

int connbuffer_read_nocopy(struct connbuffer_t* connbuffer, int len)
{
    if(!connbuffer || len < 0)
        return -1;

    if(len > connbuffer_read_len(connbuffer))
        return connbuffer_read_nocopy(connbuffer, connbuffer_read_len(connbuffer));

    connbuffer->read_pos += len;
    assert(connbuffer->write_pos >= connbuffer->read_pos);

    /* check drift threshold */
    if(connbuffer->read_pos > connbuffer->drift_threshold)
    {
        const char* shift = connbuffer_read_buffer(connbuffer);
        /* threshold is half size, so memcpy src & dst will never overlap to escape memmove */
        memcpy(connbuffer->buffer, shift, connbuffer->write_pos - connbuffer->read_pos);
        connbuffer->write_pos -= connbuffer->read_pos;
        connbuffer->read_pos = 0;
    }

    return len;
}

/*
* only read and not pick out
* return: read bytes
*/
int connbuffer_peek(struct connbuffer_t* connbuffer, char* dest, int len)
{
    if(!connbuffer || len < 0 || !dest)
        return -1;
    if(0 == len)
        return 0;
    if(len > connbuffer_read_len(connbuffer))
        return connbuffer_peek(connbuffer, dest, connbuffer_read_len(connbuffer));
    memcpy(dest, connbuffer_read_buffer(connbuffer), len);
    return len;
}

/*
* write data from src to connbuffer
* return: write bytes
*/
int connbuffer_write(struct connbuffer_t* connbuffer, const char* src, int len)
{
    if(!connbuffer || !src || len < 0)
        return -1;
    if(0 == len)
        return 0;
    if(connbuffer_write_len(connbuffer) < len)
        return connbuffer_write(connbuffer, src, connbuffer_write_len(connbuffer));
    memcpy(connbuffer_write_buffer(connbuffer), src, len);
    return connbuffer_write_nocopy(connbuffer, len);
}

int connbuffer_write_nocopy(struct connbuffer_t* connbuffer, int len)
{
    if(connbuffer_write_len(connbuffer) < len)
        return connbuffer_write_nocopy(connbuffer, connbuffer_write_len(connbuffer));
    if(len < 0)
        return -1;
    connbuffer->write_pos += len;
    assert(connbuffer->write_pos <= connbuffer->buffer_size);
    return len;
}

int connbuffer_reset(struct connbuffer_t* connbuffer)
{
    if(!connbuffer)
        return -1;
    connbuffer->read_pos = connbuffer->write_pos = 0;
    return 0;
}

const char* connbuffer_debug(struct connbuffer_t* connbuffer)
{
    static char debug_str[64];

    if(!connbuffer)
        return NULL;
    memset(debug_str, 0, sizeof(debug_str));
    snprintf(debug_str, sizeof(debug_str),
        "size=%d, read_pos=%d, write_pos=%d",
        connbuffer->buffer_size,
        connbuffer->read_pos,
        connbuffer->write_pos);
    return debug_str;
}

char* connbuffer_read_buffer(struct connbuffer_t* connbuffer)
{
    return connbuffer->buffer + connbuffer->read_pos;
}

char* connbuffer_write_buffer(struct connbuffer_t* connbuffer)
{
    return connbuffer->buffer + connbuffer->write_pos;
}

int connbuffer_read_len(struct connbuffer_t* connbuffer)
{
    if(!connbuffer)
        return -1;
    return connbuffer->write_pos > connbuffer->read_pos ?
        (connbuffer->write_pos - connbuffer->read_pos) : 0;
}

int connbuffer_write_len(struct connbuffer_t* connbuffer)
{
    if(!connbuffer)
        return -1;
    return connbuffer->buffer_size - connbuffer->write_pos;
}

