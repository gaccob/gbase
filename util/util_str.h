#ifndef UTIL_STR_H_
#define UTIL_STR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "core/os_def.h"

uint32_t util_str2int(const char* key);

#define TOLOWER(c) (char)((c >= 'A' && c <= 'Z') ? (c | 0x20) : c)
#define TOUPPER(c) (char)((c >= 'a' && c <= 'z') ? (c & ~0x20) : c)

#if defined(OS_WIN)

    #if !defined snprintf
    #define snprintf _snprintf
    #endif

    #if !defined vsnprintf
    #define vsnprintf _vsnprintf
    #endif

    inline int strcasecmp(const char *s1, const char *s2)
    {
       while  (TOUPPER((unsigned char)*s1) == TOUPPER((unsigned char)*s2++))
           if (*s1++ == 0) return 0;
       return(TOUPPER((unsigned char)*s1) - TOUPPER((unsigned char)*--s2));
    }
    inline int strncasecmp(const char *s1, const char *s2, register int n)
    {
      while (--n >= 0 && TOUPPER((unsigned char)*s1) == TOUPPER((unsigned char)*s2++))
          if (*s1++ == 0)  return 0;
      return(n < 0 ? 0 : TOUPPER((unsigned char)*s1) - TOUPPER((unsigned char)*--s2));
    }

#elif defined(OS_LINUX) || defined(OS_MAC)

#endif

enum {
    UTIL_ESCAPE_URI = 0,
    UTIL_ESCAPE_ARGS,
    UTIL_ESCAPE_URI_COMPONENT,
    UTIL_ESCAPE_URL,
    UTIL_ESCAPE_MAIL,
    UTIL_ESCAPE_MAX,
};

// dst == NULL, only return calculated escape-size
// dst != NULL, do escape and return escape-size
size_t util_uri_escape(char* dst, const char* src, size_t sz, int32_t type);
void util_uri_unescape(char** dst, char** src, size_t sz);

// dst == NULL, only return calculated escape-size
// dst != NULL, do escape and return escape-size
size_t util_html_escape(char* dst, const char* src, size_t sz);

void util_hex_dump(char* dst, char* src, size_t sz);

#ifdef __cplusplus
}
#endif

#endif // UTIL_STR_H_

