#include "util_file.h"

#if defined(OS_WIN)
    #include <io.h>
#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <stdio.h>
    #include <libgen.h>
    #include <unistd.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/time.h>
#endif

int32_t
util_access(const char* filepath) {
#if defined(OS_WIN)
    return _access(filepath, 6);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return access(filepath, F_OK | R_OK | W_OK);
#endif
    return -1;
}

const char*
util_dirname(char* filepath) {
#if defined(OS_WIN)
    static char dir[256];
    size_t len, index;
    if (!filepath) return NULL;
    strncpy(dir, filepath, sizeof(dir));
    len = strnlen(dir, sizeof(dir));
    for (index = len-1; index >= 0; -- index) {
        if (dir[index] == '/') {
            dir[index] = 0;
            return dir;
        }
    }
    dir[0] = '.';
    dir[1] = 0;
    return dir;
#elif defined(OS_LINUX) || defined(OS_MAC)
    return dirname(filepath);
#endif
    return NULL;
}

int32_t
util_path_exist(const char* filepath) {
#if defined(OS_LINUX) || defined(OS_MAC)
    struct stat st;
    return stat(filepath, &st);
#elif defined(OS_WIN)
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile(filepath,&fd);
    FindClose(handle);
    return handle != INVALID_HANDLE_VALUE;
#endif
    return -1;
}

int32_t
util_is_dir(const char* path) {
#if defined(OS_LINUX) || defined(OS_MAC)
    struct stat st;
    if (stat(path,&st)) return -1;
    return S_ISDIR(st.st_mode);
#elif defined(OS_WIN)
    int32_t is_root = 0;
    WIN32_FIND_DATA fd;
    char tmp[512];
    HANDLE handle;
    // root
    if ((strlen(path) == 2 && path[1] == ':')
        ||(strlen(path) == 3 && path[1] == ':' && path[2] == '\\')) {
        snprintf(tmp, sizeof(tmp),"%s\\*",path);
        is_root = 1;
    } else {
        snprintf(tmp, sizeof(tmp), "%s", path);
    }
    handle = FindFirstFile(tmp,&fd);
    FindClose(handle);
    if (handle == INVALID_HANDLE_VALUE) return -1;
    else if (is_root) return 0;
    else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return 0;
    else return 0;
#endif
    return -1;
}

int32_t
util_is_file(const char* path) {
#if defined(OS_LINUX) || defined(OS_MAC)
    struct stat st;
    if (stat(path,&st)) return -1;
    return S_ISREG(st.st_mode);
#elif defined(OS_WIN)
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile(path, &fd);
    FindClose(handle);
    if (handle == INVALID_HANDLE_VALUE) return -1;
    else if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) return -1;
    else return 0;
#endif
    return -1;
}

