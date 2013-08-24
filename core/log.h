#ifndef LOG_H_
#define LOG_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

#include <stdarg.h>
#include <string.h>

struct log_t;

#define ELogLevel_Fatal 0
#define ELogLevel_Error 1
#define ELogLevel_Warn 2
#define ELogLevel_Debug 3
#define ELogLevel_Trace 4

#define MAX_LOG_NAME_LEN 1024
#define MAX_LOG_BUFFER_LEN 1024
#define MAX_LOG_SIZE_ROTATE_COUNT 10

/*
*   log rotate by date
*   log_name: log file name
*   args: when ELogRotator_Size, max file size
*/
struct log_t* log_init(int log_level, const char* log_name, int args);
int log_release(struct log_t* log);
int log_write(struct log_t* log, int level, struct timeval* now, const char* file_name,
        int line_number, const char* function_name, char* fmt, ...);
int log_set_level(struct log_t* log, int log_level);

/*
*  not thread safe
*  need to guard by yourself
*/
#define LOG(log, level, now, fmt, ...) \
    do { log_write(log, level, now, __FILE__, __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__);  } while(0)

#define LOG_IMPLE(log, loglevel, fmt, ...) \
    do{ if(log) \
    { \
        struct timeval now; \
        gettimeofday(&now, NULL); \
        LOG(log, loglevel, &now, fmt, ##__VA_ARGS__); \
    } }while(0)

#define LOG_FATAL(log, fmt, ...) LOG_IMPLE(log, ELogLevel_Fatal, fmt, ##__VA_ARGS__)
#define LOG_ERROR(log, fmt, ...) LOG_IMPLE(log, ELogLevel_Error, fmt, ##__VA_ARGS__)
#define LOG_WARN(log, fmt, ...) LOG_IMPLE(log, ELogLevel_Warn, fmt, ##__VA_ARGS__)
#define LOG_DEBUG(log, fmt, ...) LOG_IMPLE(log, ELogLevel_Debug, fmt, ##__VA_ARGS__)
#define LOG_TRACE(log, fmt, ...) LOG_IMPLE(log, ELogLevel_Trace, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif // LOG_H_


