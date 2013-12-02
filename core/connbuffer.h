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
typedef void* (*connbuffer_malloc)(size_t);
typedef void (*connbuffer_free)(void*);

// buffer_size hint: 4 * max pkg size
struct connbuffer_t* connbuffer_init(int buffer_size, connbuffer_malloc buffer_malloc, connbuffer_free buffer_free);
int connbuffer_release(struct connbuffer_t* connbuffer);

int connbuffer_read(struct connbuffer_t* connbuffer, char* dest, int len);
int connbuffer_read_nocopy(struct connbuffer_t* connbuffer, int len);
int connbuffer_peek(struct connbuffer_t* connbuffer, char* dest, int len);

int connbuffer_write(struct connbuffer_t* connbuffer, const char* src, int len);
int connbuffer_write_nocopy(struct connbuffer_t* connbuffer, int len);

int connbuffer_reset(struct connbuffer_t* connbuffer);
const char* connbuffer_debug(struct connbuffer_t* connbuffer);

char* connbuffer_read_buffer(struct connbuffer_t* connbuffer);
char* connbuffer_write_buffer(struct connbuffer_t* connbuffer);

int connbuffer_read_len(struct connbuffer_t* connbuffer);
int connbuffer_write_len(struct connbuffer_t* connbuffer);

#ifdef __cplusplus
}
#endif


#endif // CONNBUFFER_H_



