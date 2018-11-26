#pragma once

#include <stdlib.h>
#include <stdint.h>

inline float randf()
{
    uint32_t x = rand() & 0xffffff;
    return (float)x / (float)0xffffff;
}

inline float randf2()
{
    return randf() * 2.0f - 1.0f;
}
