#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#include <stdarg.h>
#include <string.h>

typedef struct log_t log_t;

#define ELOG_LEVEL_FATAL 0
#define ELOG_LEVEL_ERROR 1
#define ELOG_LEVEL_WARN 2
#define ELOG_LEVEL_DEBUG 3
#define ELOG_LEVEL_TRACE 4

#define MAX_LOG_NAME_LEN 1024
#define MAX_LOG_BUFFER_LEN 1024
#define MAX_LOG_SIZE_ROTATE_COUNT 10

//  log rotate by date (other mode now not supported)
//  log_name: log file name
log_t* log_create(int level, const char* name);
int log_release(log_t* log);
int log_write(log_t* log, int level, struct timeval* now,
              const char* file, int line, const char* func,
              char* fmt, ...);
int log_set_level(log_t* log, int level);

// not thread safe
// need to guard by yourself
#define LOG(log, level, now, fmt, ...) \
    do { log_write(log, level, now, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);  } while(0)

#define LOG_IMPL(log, loglevel, fmt, ...) \
    do { if (log) \
    { \
        struct timeval now; \
        gettimeofday(&now, NULL); \
        LOG(log, loglevel, &now, fmt, ##__VA_ARGS__); \
    } } while(0)

#define lfatel(log, fmt, ...) LOG_IMPL(log, ELOG_LEVEL_FATAL, fmt, ##__VA_ARGS__)
#define lerror(log, fmt, ...) LOG_IMPL(log, ELOG_LEVEL_ERROR, fmt, ##__VA_ARGS__)
#define lwarn(log, fmt, ...) LOG_IMPL(log, ELOG_LEVEL_WARN, fmt, ##__VA_ARGS__)
#define ldebug(log, fmt, ...) LOG_IMPL(log, ELOG_LEVEL_DEBUG, fmt, ##__VA_ARGS__)
#define ltrace(log, fmt, ...) LOG_IMPL(log, ELOG_LEVEL_TRACE, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // LOG_H_


