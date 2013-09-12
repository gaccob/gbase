#include <ctype.h>
#include "util_str.h"

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

