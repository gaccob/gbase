#include "os/util.h"
#include <string.h>
#include <assert.h>
#include <ctype.h>

#define TEA_LOOP 8
#define TEA_DELTA 0x9e3779b9

#define CRC32C(c,d) (c=(c>>8)^crc_c[(c^(d))&0xFF])

static const int32_t crc_c[256] =
{
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};

int32_t util_crc32(const char* buffer, size_t length)
{
    size_t i = 0;
    int32_t crc32 = ~0L;

    for (i = 0; i < length; i++)
    {
        CRC32C(crc32, (unsigned char)buffer[i]);
    }

    return ~crc32;
}

/* src, dest: 8 bytes, key: 16 bytes */
void util_encrypt_unit(const char* src, char* dest, const char* key)
{
    uint32_t sum = 0;
    uint32_t n = TEA_LOOP;
    uint32_t src_0 = *(uint32_t*)&src[0];
    uint32_t src_1 = *(uint32_t*)&src[4];
    const uint32_t* key_int = (const uint32_t*)key;
    while (n-- > 0)
    {
        sum += TEA_DELTA;
        src_0 += ((src_1 << 4) + key_int[0]) ^ (src_1 + sum) ^ ((src_1 >> 5) + key_int[1]);
        src_1 += ((src_0 << 4) + key_int[2]) ^ (src_0 + sum) ^ ((src_0 >> 5) + key_int[3]);
    }

    *(uint32_t*)(&dest[0]) = src_0;
    *(uint32_t*)(&dest[4]) = src_1;
}

/* src, dest: 8bytes, key: 16 bytes */
void util_decrypt_unit(const char* src, char* dest, const char* key)
{
    uint32_t n = TEA_LOOP;
    uint32_t sum = (TEA_DELTA  << ILOG2(TEA_LOOP));
    uint32_t src_0 = *(uint32_t*)&src[0];
    uint32_t src_1 = *(uint32_t*)&src[4];
    const uint32_t* key_int = (const uint32_t*)key;
    while (n-- > 0)
    {
        src_1 -= ((src_0 << 4) + key_int[2]) ^ (src_0 + sum) ^ ((src_0 >> 5) + key_int[3]);
        src_0 -= ((src_1 << 4) + key_int[0]) ^ (src_1 + sum) ^ ((src_1 >> 5) + key_int[1]);
        sum -= TEA_DELTA;
    }
    *(uint32_t*)(&dest[0]) = src_0;
    *(uint32_t*)(&dest[4]) = src_1;
}


/* tea encrypt */
int32_t util_encrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len)
{
    uint64_t align_add;
    uint32_t src_floor, src_mod, i;
    char key_str[16];
    assert(src && dst && dst_len);

    /* align by 8*/
    align_add = 0;
    src_floor = src_len / 8 * 8;
    src_mod = src_len - src_floor;
    if(src_mod > 0)
    {
        memcpy((char*)&align_add, &src[src_floor], src_mod);
    }

    /* check dst len */
    if(*dst_len < src_mod + src_len + 8)
        return -1;

    /* encrypt: 8bytes head(pad) + tea encrypt */
    *(uint32_t*)dst = (src_mod > 0 ? 8 - src_mod : 0);

    /* gen 16 bytes key */
    *(uint32_t*)key_str = key;
    *(uint32_t*)(key_str + 4) = (key ^ (key >> 1));
    *(uint32_t*)(key_str + 8) = (key ^ (key >> 2));
    *(uint32_t*)(key_str + 12) = (key ^ (key >> 3));

    for(i=0; i<src_floor; i+=8)
        util_encrypt_unit(src + i, dst + i + 8, key_str);
    if(align_add > 0)
    {
        util_encrypt_unit((char*)&align_add, dst + src_floor + 8, key_str);
        *dst_len = src_floor + 8 + 8;
    }
    else
    {
        *dst_len = src_floor + 8;
    }

    return 0;
}

/* tea descrypt */
int32_t util_descrypt(const char* src, size_t src_len, int32_t key, char* dst, size_t* dst_len)
{
    char key_str[16];
    uint32_t pad;
    size_t i;

    assert(src && dst && dst_len);
    assert((src_len % 8 == 0) && (src_len > 8));

    if(*dst_len < src_len - 8)
        return -1;

    /* gen 16 bytes key */
    *(uint32_t*)key_str = key;
    *(uint32_t*)(key_str + 4) = (key ^ (key >> 1));
    *(uint32_t*)(key_str + 8) = (key ^ (key >> 2));
    *(uint32_t*)(key_str + 12) = (key ^ (key >> 3));

    /* head */
    pad = *(uint32_t*)src;

    /* descrypt by unit */
    for(i=8; i<src_len; i+=8)
        util_decrypt_unit(src + i, dst + i - 8, key_str);
    *dst_len = src_len - 8 - pad;

    return 0;
}

uint32_t util_hour_number(time_t time)
{
    struct tm now_tm;
    util_localtime(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 1000000
        + (uint32_t)(now_tm.tm_mon + 1) * 10000
        + (uint32_t)now_tm.tm_mday * 100
        + (uint32_t)now_tm.tm_hour;
}

uint32_t util_date_number(time_t time)
{
    struct tm now_tm;
    util_localtime(&time, &now_tm);
    return (uint32_t)(now_tm.tm_year + 1900) * 10000
        + (uint32_t)(now_tm.tm_mon + 1) * 100
        + (uint32_t)now_tm.tm_mday;
}

void util_timestamp(struct timeval* time, char* stamp)
{
    struct tm now_tm;
    util_localtime((time_t*)&time->tv_sec, &now_tm);
    sprintf(stamp, "[%d-%02d-%02d %02d:%02d:%02d:%06d]",
        now_tm.tm_year + 1900,
        now_tm.tm_mon + 1,
        now_tm.tm_mday,
        now_tm.tm_hour,
        now_tm.tm_min,
        now_tm.tm_sec,
        (uint32_t)time->tv_usec);
}

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

int32_t util_access(const char* filepath)
{
#if defined(OS_WIN)
    return _access(filepath, 6);
#elif defined(OS_LINUX) || defined(OS_MAC)
    return access(filepath, F_OK | R_OK | W_OK);
#endif
    return -1;
}

void util_localtime(const time_t *time, struct tm* _tm)
{
#if defined(OS_WIN)
    localtime_s(_tm, time);
#elif defined(OS_LINUX) || defined(OS_MAC)
    localtime_r(time, _tm);
#endif
}

void util_gettimeofday(struct timeval* tv, void* tz)
{
#if defined(OS_WIN)
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;
    GetLocalTime(&wtm);
    tm.tm_year = wtm.wYear - 1900;
    tm.tm_mon = wtm.wMonth - 1;
    tm.tm_mday = wtm.wDay;
    tm.tm_hour = wtm.wHour;
    tm.tm_min = wtm.wMinute;
    tm.tm_sec = wtm.wSecond;
    tm.tm_isdst = -1;
    clock = mktime(&tm);
    tv->tv_sec = (long)clock;
    tv->tv_usec = wtm.wMilliseconds * 1000;
#elif defined(OS_LINUX) || defined(OS_MAC)
    gettimeofday(tv, (struct timezone*)tz);
#endif
}

/* tv1 and tv2 must be normalized */
int32_t util_time_compare(struct timeval* tv1, struct timeval* tv2)
{
    assert(tv1 && tv2);
    if(tv1->tv_sec == tv2->tv_sec && tv1->tv_usec == tv2->tv_usec)
        return 0;

    if(tv1->tv_sec > tv2->tv_sec
        || (tv1->tv_sec == tv2->tv_sec
        && tv1->tv_usec > tv2->tv_usec))
        return 1;

    return -1;
}

void util_time_add(struct timeval* tv1, struct timeval* tv2, struct timeval* sum)
{
    assert(tv1 && tv2 && sum);
    sum->tv_sec = tv1->tv_sec + tv2->tv_sec;
    sum->tv_usec = tv1->tv_usec + tv2->tv_usec;
    while(sum->tv_usec > 1000000)
    {
        sum->tv_sec ++;
        sum->tv_usec -= 1000000;
    }
}

void util_time_sub(struct timeval* tv1, struct timeval* tv2, struct timeval* sub)
{
    assert(tv1 && tv2 && sub);
    assert(util_time_compare(tv1, tv2) > 0);
    sub->tv_sec = tv1->tv_sec - tv2->tv_sec;
    if(tv1->tv_usec < tv2->tv_usec)
    {
        sub->tv_usec = tv1->tv_usec + 1000000 - tv2->tv_usec;
        sub->tv_sec --;
    }
    else
    {
        sub->tv_usec = tv1->tv_usec - tv2->tv_usec;
    }
}

const char* util_dirname(char* filepath)
{
#if defined(OS_WIN)
    static char dir[256];
    size_t len, index;

    if(!filepath)
        return NULL;
    strncpy(dir, filepath, sizeof(dir));
    len = strnlen(dir, sizeof(dir));
    for(index = len-1; index >= 0; index --)
    {
        if(dir[index] == '/')
        {
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

int32_t util_path_exist(const char* filepath)
{
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

int32_t util_is_dir(const char* path)
{
#if defined(OS_LINUX) || defined(OS_MAC)
    struct stat st;
    if(stat(path,&st)) return -1;
    return S_ISDIR(st.st_mode);

#elif defined(OS_WIN)
    int32_t is_root = 0;
    WIN32_FIND_DATA fd;
    char tmp[512];
    HANDLE handle;

    /* root */
    if((strlen(path) == 2 && path[1] == ':')
        ||(strlen(path) == 3 && path[1] == ':' && path[2] == '\\'))
    {
        snprintf(tmp, sizeof(tmp),"%s\\*",path);
        is_root = 1;
    }
    else
        snprintf(tmp, sizeof(tmp), "%s", path);

    handle = FindFirstFile(tmp,&fd);
    FindClose(handle);
    if(handle == INVALID_HANDLE_VALUE)
        return -1;
    /* root */
    else     if(is_root)
        return 0;
    /* is direectory */
    else if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return 0;
    /* is file */
    else
        return 0;
#endif
    return -1;
}

int32_t util_is_file(const char* path)
{
#if defined(OS_LINUX) || defined(OS_MAC)
    struct stat st;
    if(stat(path,&st))
        return -1;
    return S_ISREG(st.st_mode);

#elif defined(OS_WIN)
    WIN32_FIND_DATA fd;
    HANDLE handle = FindFirstFile(path, &fd);
    FindClose(handle);
    if(handle == INVALID_HANDLE_VALUE)
        return -1;
    else if(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        return -1;
    else
        return 0;

#endif
    return -1;
}

const char _base64_table[64] =
{
    'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
    'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
    'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
    'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
    'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
    'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
    'w', 'x', 'y', 'z', '0', '1', '2', '3',
    '4', '5', '6', '7', '8', '9', '+', '/'
};

#define _BASE64_ENCODE(a4, b3) \
    a4[0] = (b3[0] & 0xfc) >> 2; \
    a4[1] = ((b3[0] & 0x03) << 4) | ((b3[1] & 0xf0) >> 4); \
    a4[2] = ((b3[1] & 0x0f) << 2) | ((b3[2] & 0xc0) >> 6); \
    a4[3] = b3[2] & 0x3f;

int32_t util_base64_encode(char* dst, const char* src, size_t sz)
{
    size_t idx = 0, i = 0, j;
    uint8_t a[4], b[3];
    if (!dst || !src || sz == 0)
        return -1;
    while (sz-- > 0 && src[idx] != 0)
    {
        b[i++] = src[idx++];
        if (i == 3)
        {
            _BASE64_ENCODE(a, b);
            for (j = 0; j < 4; j ++)
                *dst++ = _base64_table[a[j]];
            i = 0;
        }
    }
    if (i > 0)
    {
        for (j = i; j < 3; j++)
            b[j] = '\0';
        _BASE64_ENCODE(a, b);
        for (j = 0; j < i + 1; j++)
            *dst++ = _base64_table[a[j]];
        while (i++ < 3)
            *dst++ = '=';
    }
    *dst++ = 0;
    return 0;
}

#define _BASE64_C(c) (isalnum(c) || ((c) == '+') || ((c) == '/'))
#define _BASE64_DECODE(a3, b4) \
    a3[0] = (b4[0] << 2) + ((b4[1] & 0x30) >> 4); \
    a3[1] = ((b4[1] & 0x0f) << 4) + ((b4[2] & 0x3c) >> 2); \
    a3[2] = ((b4[2] & 0x03) << 6) + b4[3];

int32_t util_base64_decode(char* dst, const char* src, size_t sz)
{
    if (!dst || !src || sz == 0)
        return -1;
    size_t i = 0, idx = 0, j;
    uint8_t a[3], b[4];
    while (sz-- > 0 && src[idx] != '=')
    {
        if (!_BASE64_C(src[idx]))
            return -1;
        b[i++] = src[idx++];
        if (i == 4)
        {
            for (j = 0; j < i; j++)
                b[j] = strchr(_base64_table, b[j]) - _base64_table;
            _BASE64_DECODE(a, b);
            for (j = 0; j < 3; j++)
                *dst++ = a[j];
            i = 0;
        }
    }
    if (i > 0)
    {
        for (j = i; j < 4; j++)
            b[j] = 0;
        for (j = 0; j < 4; j++)
            b[j] = strchr(_base64_table, b[j]) - _base64_table;
        _BASE64_DECODE(a, b);
        for (j = 0; j < i - 1; j++)
            *dst++ = a[j];
    }
    *dst++ = 0;
    return 0;
}

uint32_t util_str2int(const char *key)
{
    char res_decimals[15] = "";
    char *tail_res = res_decimals;
    uint8_t space_count = 0;
    uint8_t i = 0;
    do {
        if (isdigit(key[i]))
            strncat(tail_res++, &key[i], 1);
        if (key[i] == ' ')
            space_count++;
    } while (key[++i]);    
    return ((uint32_t) strtoul(res_decimals, NULL, 10) / space_count);
}

