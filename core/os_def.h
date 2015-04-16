#ifndef OS_DEF_H_
#define OS_DEF_H_

//
// define OS and some common types
// now support LINUX, MAC
//

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// win32 & msc_ver
#if defined(WIN32) || defined(_WIN32)
    #error ignore boring WIN32

// linux & gnuc
#elif defined(__LINUX__) || defined(__linux__)
    #define OS_LINUX 1

// mac & gnuc
#elif defined(__APPLE__) || defined(__APPLE_CC__)
    #define OS_MAC 1

// cygwin
#elif defined(__CYGWIN__)
    #define OS_CYGWIN 1

// other os
#else
    #error other platform not support now
#endif

// gcc version
#ifndef __GNUC__
    #error no GNUC
#endif

// gcc version
#if defined(__GNUC__)
    #if !defined GCC_VERSION
        #define GCC_VERSION    (__GNUC__ * 10000 +  __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
    #endif
#endif

// endian type
#if defined(__sparc) || defined(__sparc__) \
    || defined(_POWER) || defined(__powerpc__) \
    || defined(__ppc__) || defined(__hppa) \
    || defined(_MIPSEB) || defined(_POWER) \
    || defined(__s390__)
# define OS_BIG_ENDIAN
#elif defined(__i386__) || defined(__alpha__) \
    || defined(__ia64) || defined(__ia64__) \
    || defined(_M_IX86) || defined(_M_IA64) \
    || defined(_M_ALPHA) || defined(__x86_64__)
# define OS_LITTLE_ENDIAN
#else
# error endian type not recognized
#endif

#if !defined ILOG2
#define ILOG2(n) \
    ( \
                 (n) & (1ULL << 63) ? 63 :       \
                 (n) & (1ULL << 62) ? 62 :       \
                 (n) & (1ULL << 61) ? 61 :       \
                 (n) & (1ULL << 60) ? 60 :       \
                 (n) & (1ULL << 59) ? 59 :       \
                 (n) & (1ULL << 58) ? 58 :       \
                 (n) & (1ULL << 57) ? 57 :       \
                 (n) & (1ULL << 56) ? 56 :       \
                 (n) & (1ULL << 55) ? 55 :       \
                 (n) & (1ULL << 54) ? 54 :       \
                 (n) & (1ULL << 53) ? 53 :       \
                 (n) & (1ULL << 52) ? 52 :       \
                 (n) & (1ULL << 51) ? 51 :       \
                 (n) & (1ULL << 50) ? 50 :       \
                 (n) & (1ULL << 49) ? 49 :       \
                 (n) & (1ULL << 48) ? 48 :       \
                 (n) & (1ULL << 47) ? 47 :       \
                 (n) & (1ULL << 46) ? 46 :       \
                 (n) & (1ULL << 45) ? 45 :       \
                 (n) & (1ULL << 44) ? 44 :       \
                 (n) & (1ULL << 43) ? 43 :       \
                 (n) & (1ULL << 42) ? 42 :       \
                 (n) & (1ULL << 41) ? 41 :       \
                 (n) & (1ULL << 40) ? 40 :       \
                 (n) & (1ULL << 39) ? 39 :       \
                 (n) & (1ULL << 38) ? 38 :       \
                 (n) & (1ULL << 37) ? 37 :       \
                 (n) & (1ULL << 36) ? 36 :       \
                 (n) & (1ULL << 35) ? 35 :       \
                 (n) & (1ULL << 34) ? 34 :       \
                 (n) & (1ULL << 33) ? 33 :       \
                 (n) & (1ULL << 32) ? 32 :       \
                 (n) & (1ULL << 31) ? 31 :       \
                 (n) & (1ULL << 30) ? 30 :       \
                 (n) & (1ULL << 29) ? 29 :       \
                 (n) & (1ULL << 28) ? 28 :       \
                 (n) & (1ULL << 27) ? 27 :       \
                 (n) & (1ULL << 26) ? 26 :       \
                 (n) & (1ULL << 25) ? 25 :       \
                 (n) & (1ULL << 24) ? 24 :       \
                 (n) & (1ULL << 23) ? 23 :       \
                 (n) & (1ULL << 22) ? 22 :       \
                 (n) & (1ULL << 21) ? 21 :       \
                 (n) & (1ULL << 20) ? 20 :       \
                 (n) & (1ULL << 19) ? 19 :       \
                 (n) & (1ULL << 18) ? 18 :       \
                 (n) & (1ULL << 17) ? 17 :       \
                 (n) & (1ULL << 16) ? 16 :       \
                 (n) & (1ULL << 15) ? 15 :       \
                 (n) & (1ULL << 14) ? 14 :       \
                 (n) & (1ULL << 13) ? 13 :       \
                 (n) & (1ULL << 12) ? 12 :       \
                 (n) & (1ULL << 11) ? 11 :       \
                 (n) & (1ULL << 10) ? 10 :       \
                 (n) & (1ULL <<  9) ?  9 :       \
                 (n) & (1ULL <<  8) ?  8 :       \
                 (n) & (1ULL <<  7) ?  7 :       \
                 (n) & (1ULL <<  6) ?  6 :       \
                 (n) & (1ULL <<  5) ?  5 :       \
                 (n) & (1ULL <<  4) ?  4 :       \
                 (n) & (1ULL <<  3) ?  3 :       \
                 (n) & (1ULL <<  2) ?  2 :       \
                 (n) & (1ULL <<  1) ?  1 :       \
                 (n) & (1ULL <<  0) ?  0 : 0     \
    )
#endif

// get 2^n round-up
#if !defined ROUNDUP
#define ROUNDUP(n)  ((n == 1) ? 1 : (1 << (ILOG2(n - 1) + 1)))
#endif

// get 2^n round-down
#if !defined ROUNDDOWN
#define ROUNDDOWN(n) (1 << (ILOG2(n)))
#endif

// round x up by y, y = 2^n
#if !defined ROUNDUP2
#define ROUNDUP2(X, Y) (((X) + ((Y) - 1)) & ~((Y) - 1))
#endif
// round y down by y, y = 2^n
#ifndef ROUNDDOWN2
#define ROUNDDOWN2(X, Y) ((X) & -(Y))
#endif

// check is power of 2
#if !defined ROUND2
#define ROUND2(n) ((n) & ((n)-1))
#endif

#if !defined MALLOC
#define MALLOC malloc
#endif

#if !defined FREE
#define FREE free
#endif

#if !defined REALLOC
#define REALLOC realloc
#endif

#if !defined VALLOC
#define VALLOC valloc
#endif

#ifdef __cplusplus
}
#endif

#endif // OS_DEF_H_
