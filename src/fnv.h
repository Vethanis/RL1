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

inline u32 Fnv32(CByteSlice bytes)
{
    u32 y = 2166136261u;
    for(u8 byte : bytes)
    {
        y ^= byte;
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

inline u64 Fnv64(CByteSlice bytes)
{
    u64 y = 14695981039346656037ull;
    for(u8 byte : bytes)
    {
        y ^= byte;
        y *= 1099511628211ull;
    }
    return y;
}
