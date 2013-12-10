#include <assert.h>
#include <fcntl.h>
#include "util/util_file.h"
#include "util/util_time.h"
#include "log.h"

#if defined(OS_WIN)
#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <dirent.h>
#endif

typedef struct log_rotator_t
{
    time_t log_start_time;

    // current log file
    int fd;
    char log_file[MAX_LOG_NAME_LEN];
    uint32_t log_suffix;
    size_t log_file_size;

    // rotate function
    int (*check_and_rotate)(struct log_t*);
} log_rotator_t;

typedef struct log_t
{
    log_rotator_t* rotator;
    int log_level;
    char log_name[MAX_LOG_NAME_LEN];
} log_t;


int log_do_date_rotate(log_t* log)
{
    time_t now = time(NULL);
    assert(log);
    if (now >= log->rotator->log_start_time + 60 * 60 * 24) {
        log->rotator->log_suffix = util_date_number(now);
        sprintf(log->rotator->log_file, "%s.%u", log->log_name, log->rotator->log_suffix);
        log->rotator->fd = open(log->rotator->log_file, O_CREAT | O_RDWR | O_APPEND, 0666);
        if (log->rotator->fd < 0) {
            fprintf(stderr, "open log file[%s] fail\n", log->rotator->log_file);
            return -1;
        }
    }
    return 0;
}

int log_date_rotator_init(log_t* log)
{
    time_t now;
    struct tm now_tm;
    assert(log && log->rotator);

    // log rotator type
    log->rotator->check_and_rotate = log_do_date_rotate;

    // log start time
    now = time(NULL);
    util_localtime(&now, &now_tm);
    now_tm.tm_hour = 0;
    now_tm.tm_min  = 0;
    now_tm.tm_sec  = 0;
    log->rotator->log_start_time = mktime(&now_tm);

    // log file name
    log->rotator->log_suffix = util_date_number(now);
    sprintf(log->rotator->log_file, "%s.%u", log->log_name, log->rotator->log_suffix);

    // open log file
    if (0 == util_access(log->rotator->log_file)) {
        log->rotator->fd = open(log->rotator->log_file, O_RDWR | O_APPEND, 0666);
    } else {
        log->rotator->fd = open(log->rotator->log_file, O_CREAT | O_RDWR | O_APPEND, 0666);
    }
    if (log->rotator->fd < 0) {
        fprintf(stderr, "open log file[%s] fail\n", log->rotator->log_file);
        return -1;
    }
    return 0;
}


//  log rotate by date
//  log_name: log file name
//  args: when ELogRotator_Size, max file size
log_t* log_init(int log_level, const char* log_name, int args)
{
    log_t* log;
    if (!log_name) return NULL;
    log = (log_t*)MALLOC(sizeof(log_t));
    assert(log);

    log->log_level = log_level;
    snprintf(log->log_name, MAX_LOG_NAME_LEN, "%s", log_name);
    log->rotator = (log_rotator_t*)MALLOC(sizeof(log_rotator_t));
    assert(log->rotator);

    if (log_date_rotator_init(log) < 0)
        goto FAIL;
    return log;

FAIL:
    FREE(log->rotator);
    FREE(log);
    return NULL;
}

int log_release(log_t* log)
{
    if (log) {
        FREE(log->rotator);
        FREE(log);
    }
    return 0;
}

int log_write(log_t* log, int level, struct timeval* now,
              const char* file_name, int line_number,
              const char* function_name, char* fmt, ...)
{
    char buffer[MAX_LOG_BUFFER_LEN];
    size_t content_len;
    va_list ap;

    if (level <= log->log_level) {
        // timestamp & file line function tag
        util_timestamp(now, buffer, MAX_LOG_BUFFER_LEN);
        snprintf(buffer + strlen(buffer), MAX_LOG_BUFFER_LEN - 1 - strlen(buffer),
            "[%s:%d:%s] \t", file_name, line_number, function_name);

        // log content
        va_start(ap,fmt);
        vsnprintf(buffer + strlen(buffer), MAX_LOG_BUFFER_LEN - 1 - strlen(buffer), fmt, ap);
        va_end(ap);
        strcat(buffer, "\n");

        // add log file size
        content_len = strlen(buffer);

        // write log
        if (write(log->rotator->fd, buffer, content_len)==-1) {
            fprintf(stderr, "write log[%s] fail. %s\n", buffer, strerror(errno));
            return -1;
        }

        // rotate check
        log->rotator->check_and_rotate(log);
    }
    return 0;
}

int log_set_level(log_t* log, int log_level)
{
    if (log) {
        log->log_level = log_level;
        return 0;
    }
    fprintf(stderr, "set loglevel, log invalid fail.\n");
    return -1;
}

