#ifndef CONNBUFFER_H_
#define CONNBUFFER_H_

//
// buffer for connection, read / write like pop / push
//

#ifdef __cplusplus
extern "C" {
#endif

#include <assert.h>
#include "core/os_def.h"

struct connbuffer_t;
typedef struct connbuffer_t connbuffer_t;

typedef void* (*connbuffer_malloc)(size_t);
typedef void (*connbuffer_free)(void*);

// buffer_size hint: 4 * max pkg size
connbuffer_t* connbuffer_create(int buffer_size,
                                connbuffer_malloc malloc_func,
                                connbuffer_free free_func);

int connbuffer_release(connbuffer_t*);

int connbuffer_read(connbuffer_t*, char* dest, int len);
int connbuffer_read_nocopy(connbuffer_t*, int len);
int connbuffer_peek(connbuffer_t*, char* dest, int len);

int connbuffer_write(connbuffer_t*, const char* src, int len);
int connbuffer_write_nocopy(connbuffer_t*, int len);

int connbuffer_reset(connbuffer_t*);
const char* connbuffer_debug(connbuffer_t*);

char* connbuffer_read_buffer(connbuffer_t*);
char* connbuffer_write_buffer(connbuffer_t*);

int connbuffer_read_len(connbuffer_t*);
int connbuffer_write_len(connbuffer_t*);

#ifdef __cplusplus
}
#endif


#endif // CONNBUFFER_H_



