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

typedef struct log_rotator_t {
    time_t start;
    int fd;
    char file[MAX_LOG_NAME_LEN];
    uint32_t suffix;
    size_t size;
    // rotate function
    int (*check_and_rotate)(struct log_t*);
} log_rotator_t;

typedef struct log_t {
    log_rotator_t* rotator;
    int level;
    char name[MAX_LOG_NAME_LEN];
} log_t;

#define LOG_DATE_SECONDS (60 * 60 * 24)

static int
_date_rotate(log_t* log) {
    time_t now = time(NULL);
    assert(log);
    if (now >= log->rotator->start + LOG_DATE_SECONDS) {
        log->rotator->suffix = util_date_number(now);
        sprintf(log->rotator->file, "%s.%u", log->name, log->rotator->suffix);
        log->rotator->fd = open(log->rotator->file, O_CREAT | O_RDWR | O_APPEND, 0666);
        return log->rotator->fd < 0 ? -1 : 0;
    }
    return 0;
}

static int
_log_date_rotator_create(log_t* log) {
    time_t now;
    struct tm now_tm;
    assert(log && log->rotator);
    log->rotator->check_and_rotate = _date_rotate;
    now = time(NULL);
    util_localtime(&now, &now_tm);
    now_tm.tm_hour = 0;
    now_tm.tm_min  = 0;
    now_tm.tm_sec  = 0;
    log->rotator->start = mktime(&now_tm);
    log->rotator->suffix = util_date_number(now);
    sprintf(log->rotator->file, "%s.%u", log->name, log->rotator->suffix);
    if (0 == util_access(log->rotator->file)) {
        log->rotator->fd = open(log->rotator->file, O_RDWR | O_APPEND, 0666);
    } else {
        log->rotator->fd = open(log->rotator->file, O_CREAT | O_RDWR | O_APPEND, 0666);
    }
    return log->rotator->fd < 0 ? -1 : 0;
}

//  log rotate by date
//  name: log file name
log_t*
log_create(int level, const char* name) {
    log_t* log;
    if (!name) return NULL;
    log = (log_t*)MALLOC(sizeof(log_t));
    assert(log);
    log->level = level;
    snprintf(log->name, MAX_LOG_NAME_LEN, "%s", name);
    log->rotator = (log_rotator_t*)MALLOC(sizeof(log_rotator_t));
    assert(log->rotator);

    if (_log_date_rotator_create(log) < 0) {
        FREE(log->rotator);
        FREE(log);
        return NULL;
    }
    return log;
}

int
log_release(log_t* log) {
    if (log) {
        FREE(log->rotator);
        FREE(log);
    }
    return 0;
}

int log_write(log_t* log, int level, struct timeval* now, const char* name,
              int line, const char* function, char* fmt, ...)
{
    char buffer[MAX_LOG_BUFFER_LEN];
    size_t len;
    va_list ap;
    if (level <= log->level) {
        util_timestamp(now, buffer, MAX_LOG_BUFFER_LEN);
        snprintf(buffer + strlen(buffer), MAX_LOG_BUFFER_LEN - 1 - strlen(buffer),
            "[%s:%d:%s] \t", name, line, function);

        va_start(ap,fmt);
        vsnprintf(buffer + strlen(buffer), MAX_LOG_BUFFER_LEN - 1 - strlen(buffer), fmt, ap);
        va_end(ap);
        strcat(buffer, "\n");

        len = strlen(buffer);
        if (write(log->rotator->fd, buffer, len)==-1) {
            fprintf(stderr, "write log[%s] fail. %s\n", buffer, strerror(errno));
            return -1;
        }

        // rotate check
        log->rotator->check_and_rotate(log);
    }
    return 0;
}

int
log_set_level(log_t* log, int level) {
    if (log) {
        log->level = level;
        return 0;
    }
    return -1;
}

