#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include "util_file.h"

int32_t
util_access(const char* filepath) {
    return access(filepath, F_OK | R_OK | W_OK);
}

const char*
util_dirname(char* filepath) {
    return dirname(filepath);
}

int32_t
util_path_exist(const char* filepath) {
    struct stat st;
    return stat(filepath, &st);
}

int32_t
util_is_dir(const char* path) {
    struct stat st;
    if (stat(path,&st))
        return -1;
    return S_ISDIR(st.st_mode);
}

int32_t
util_is_file(const char* path) {
    struct stat st;
    if (stat(path,&st))
        return -1;
    return S_ISREG(st.st_mode);
}

