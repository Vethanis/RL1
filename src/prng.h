#pragma once

#include "pcg32.h"
#include "lang.h"

static constexpr u32 NumPrngStates = 16;

extern pcg32_random_t g_pcgstate[NumPrngStates];

inline void SRand(uint64_t value)
{
    for(u32 i = 0; i < NumPrngStates; ++i)
    {
        g_pcgstate[i].state = value ^ i;
        g_pcgstate[i].inc = 1u;
        for(u32 j = 0; j < i; ++j)
        {
            pcg32_random_r(g_pcgstate + i);
        }
    }
}

inline u32 Rand(u32 tid = 0u)
{
    constexpr u32 mask = NumPrngStates - 1u;
    pcg32_random_t* state = g_pcgstate + (tid & mask);
    return pcg32_random_r(state);
}

inline u32 Rand(u32 lo, u32 hi, u32 tid = 0u)
{
    return lo + (Rand(tid) % (hi - lo));
}

inline float Randf(u32 tid = 0u)
{
    // floats only have 24 bits of precision, thus 24 bit mask
    constexpr u32 mask = 0xFFFFFF;
    constexpr float inv = 1.0f / (float)mask;
    u32 x = Rand(tid) & mask;
    return (float)x * inv;
}

inline float Randf(float lo, float hi, u32 tid = 0u)
{
    return lo + Randf(tid) * (hi - lo);
}
