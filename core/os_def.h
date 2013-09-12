#ifndef OS_DEF_H_
#define OS_DEF_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

// win32 & msc_ver
#if defined(WIN32) || defined(_WIN32)
    #define OS_WIN 1
    #ifndef _MSC_VER
        #error WIN32 but no MSC_VER
    #endif

// linux & gnuc
#elif defined(__LINUX__) || defined(__linux__)
    #define OS_LINUX 1
    #ifndef __GNUC__
        #error LINUX but no GNUC
    #endif

// mac & gnuc
#elif defined(__APPLE__) || defined(__APPLE_CC__)
    #define OS_MAC 1
    #ifndef __GNUC__
        #error LINUX but no GNUC
    #endif

// other os
#else
    #error other platform not support now
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


#if defined(OS_WIN)
    #include <winsock2.h>

    #define ERRNO WSAGetLastError()
    #define ERR_EINTR WSAEINTR
    #define ERR_EAGAIN WSAEINPROGRESS
    #define ERR_EWOULDBLOCK WSAEWOULDBLOCK
    #define ERR_EINPROGRESS WSAEINPROGRESS

    #define SLEEP(ms) Sleep(ms)

    #include <io.h>
    #include <direct.h>
    #include <sys/types.h>
    #pragma warning(disable:4996)
    #if !defined(__cplusplus)
        #define inline __inline
    #endif
    #ifndef typeof
        #define typeof typeid;
    #endif

    typedef __int8   int8_t;
    typedef __int16  int16_t;
    typedef __int32  int32_t;
    typedef __int64  int64_t;
    typedef unsigned __int8   uint8_t;
    typedef unsigned __int16  uint16_t;
    typedef unsigned __int32  uint32_t;
    typedef unsigned __int64  uint64_t;

    #if !defined snprintf
    #define snprintf _snprintf
    #endif

    #if !defined vsnprintf
    #define vsnprintf _vsnprintf
    #endif

    inline int strcasecmp(const char *s1, const char *s2)
    {
       while  (toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
           if (*s1++ == 0) return 0;
       return(toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
    }
    inline int strncasecmp(const char *s1, const char *s2, register int n)
    {
      while (--n >= 0 && toupper((unsigned char)*s1) == toupper((unsigned char)*s2++))
          if (*s1++ == 0)  return 0;
      return(n < 0 ? 0 : toupper((unsigned char)*s1) - toupper((unsigned char)*--s2));
    }

#elif defined(OS_LINUX) || defined(OS_MAC)
    #include <arpa/inet.h>
    #include <unistd.h>

    #define ERRNO errno
    #define ERR_EINTR EINTR
    #define ERR_EAGAIN EAGAIN
    #define ERR_EWOULDBLOCK EWOULDBLOCK
    #define ERR_EINPROGRESS EINPROGRESS

    #define SLEEP(ms) usleep(ms * 1000)

    #include <stdint.h>
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

/* get 2^n round-up */
#if !defined ROUNDUP
#define ROUNDUP(n)  ((n == 1) ? 1 : (1 << (ILOG2(n - 1) + 1)))
#endif

/* get 2^n round-down */
#if !defined ROUNDDOWN
#define ROUNDDOWN(n) (1 << (ILOG2(n)))
#endif

/* check is power of 2 */
#if !defined ROUND2
#define ROUND2(n) ((n) & ((n)-1))
#endif

#if !defined MALLOC
#define MALLOC malloc
#endif

#if !defined FREE
#define FREE free
#endif

#ifdef __cplusplus
}
#endif

#endif // OS_DEF_H_
