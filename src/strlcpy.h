#pragma once

#include <stdint.h>

inline void strlcpy(char* dst, const char* src, int32_t len)
{
    int32_t i = 0;
    for(; i < len - 1; ++i)
    {
        dst[i] = src[i];
    }
    dst[i] = '\0';
}
