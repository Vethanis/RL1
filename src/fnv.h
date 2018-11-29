#pragma once

#include <stdint.h>

inline uint32_t Fnv32(const char* x)
{
    uint32_t y = 2166136261u;
    while(*x)
    {
        y ^= *x;
        y *= 16777619u;
        ++x;
    }
    return y;
}

inline uint32_t Fnv32(const void* x, uint32_t len)
{
    const uint8_t* octets = (uint8_t*)x;
    uint32_t y = 2166136261u;
    for(uint32_t i = 0u; i < len; ++i)
    {
        y ^= octets[i];
        y *= 16777619u;
    }
    return y;
}

inline uint64_t Fnv64(const char* x)
{
    uint64_t y = 14695981039346656037ull;
    while(*x)
    {
        y ^= *x;
        y *= 1099511628211ull;
        ++x;
    }
    return y;
}

inline uint64_t Fnv64(const void* x, uint32_t len)
{
    const uint8_t* octets = (uint8_t*)x;
    uint64_t y = 14695981039346656037ull;
    for(uint32_t i = 0u; i < len; ++i)
    {
        y ^= octets[i];
        y *= 1099511628211ull;
    }
    return y;
}
