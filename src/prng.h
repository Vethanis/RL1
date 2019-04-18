#pragma once

#include "pcg32.h"

static constexpr uint32_t NumPrngStates = 16;

extern pcg32_random_t g_pcgstate[NumPrngStates];

static void SRand(uint64_t value)
{
    for(uint32_t i = 0; i < NumPrngStates; ++i)
    {
        g_pcgstate[i].state = value ^ i;
        g_pcgstate[i].inc = 1u;
        for(uint32_t j = 0; j < i; ++j)
        {
            pcg32_random_r(g_pcgstate + i);
        }
    }
}

static uint32_t Rand(uint32_t tid = 0u)
{
    constexpr uint32_t mask = NumPrngStates - 1u;
    pcg32_random_t* state = g_pcgstate + (tid & mask);
    return pcg32_random_r(state);
}

static uint32_t Rand(uint32_t lo, uint32_t hi, uint32_t tid = 0u)
{
    return lo + (Rand(tid) % (hi - lo));
}

static float Randf(uint32_t tid = 0u)
{
    // floats only have 24 bits of precision, thus 24 bit mask
    constexpr uint32_t mask = 0xFFFFFF;
    constexpr float inv = 1.0f / (float)mask;
    uint32_t x = Rand(tid) & mask;
    return (float)x * inv;
}

static float Randf(float lo, float hi, uint32_t tid = 0u)
{
    return lo + Randf(tid) * (hi - lo);
}
