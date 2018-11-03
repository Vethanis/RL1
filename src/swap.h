#pragma once

#include <string.h>
#include <stdint.h>

template<typename T>
void Swap(T& a, T& b)
{
    uint64_t mem[1u + sizeof(T) / sizeof(uint64_t)];
    memcpy(mem, &a, sizeof(T));
    memcpy(&a, &b, sizeof(T));
    memcpy(&b, mem, sizeof(T));
}
