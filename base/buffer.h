#ifndef BUFFER_H_
#define BUFFER_H_

//
// buffer for connection, read / write like pop / push
//

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include "core/os_def.h"

typedef struct buffer_t buffer_t;

typedef void* (*buffer_malloc_func)(size_t);
typedef void (*buffer_free_func)(void*);

// buffer_size hint: 4 * max pkg size
buffer_t* buffer_create(int buffer_size,
                        buffer_malloc_func malloc_func,
                        buffer_free_func free_func);

int buffer_release(buffer_t*);

int buffer_read(buffer_t*, char* dest, int len);
int buffer_read_nocopy(buffer_t*, int len);
int buffer_peek(buffer_t*, char* dest, int len);

int buffer_write(buffer_t*, const char* src, int len);
int buffer_write_nocopy(buffer_t*, int len);

int buffer_reset(buffer_t*);

// not thread safe
const char* buffer_debug(buffer_t*);

char* buffer_read_buffer(buffer_t*);
char* buffer_write_buffer(buffer_t*);

int buffer_read_len(buffer_t*);
int buffer_write_len(buffer_t*);

#ifdef __cplusplus
}
#endif

#endif // BUFFER_H_

