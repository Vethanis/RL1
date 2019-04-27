#pragma once

#include "slice.h"

inline u32 Fnv32(const char* x)
{
    u32 y = 2166136261u;
    while(*x)
    {
        y ^= *x;
        y *= 16777619u;
        ++x;
    }
    return y;
}

inline u32 Fnv32(const void* ptr, usize len)
{
    u32 y = 2166136261u;
    const u8* bytes = (const u8*)ptr;
    for(usize i = 0; i < len; ++i)
    {
        y ^= bytes[i];
        y *= 16777619u;
    }
    return y;
}

inline u64 Fnv64(const char* x)
{
    u64 y = 14695981039346656037ull;
    while(*x)
    {
        y ^= *x;
        y *= 1099511628211ull;
        ++x;
    }
    return y;
}

inline u64 Fnv64(const void* ptr, usize len)
{
    u64 y = 14695981039346656037ull;
    const u8* bytes = (const u8*)ptr;
    for(usize i = 0; i < len; ++i)
    {
        y ^= bytes[i];
        y *= 1099511628211ull;
    }
    return y;
}
