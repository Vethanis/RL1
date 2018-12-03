#pragma once

#include <stdint.h>

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

uint32_t pcg32_random_r(pcg32_random_t* rng);

// -------------------------------------------------------------------
// Modifications below here are by Kara Hughes
// -------------------------------------------------------------------

enum 
{
    NumPCGStates = 16,
};

extern pcg32_random_t g_pcgstate[NumPCGStates];

inline void SRand(uint64_t value)
{
    for(uint32_t i = 0; i < NumPCGStates; ++i)
    {
        g_pcgstate[i].state = value ^ i;
        g_pcgstate[i].inc = 1u;
        for(uint32_t j = 0; j < i; ++j)
        {
            pcg32_random_r(g_pcgstate + i);
        }
    }
}

inline uint32_t Rand(uint32_t tid = 0u)
{
    constexpr uint32_t mask = NumPCGStates - 1u;
    pcg32_random_t* state = g_pcgstate + (tid & mask);
    return pcg32_random_r(state);
}

inline uint32_t Rand(uint32_t lo, uint32_t hi, uint32_t tid = 0u)
{
    return lo + (Rand(tid) % (hi - lo));
}

inline float Randf(uint32_t tid = 0u)
{
    // floats only have 24 bits of precision, thus 24 bit mask
    constexpr uint32_t mask = 0xFFFFFF;
    constexpr float inv = 1.0f / (float)mask;
    uint32_t x = Rand(tid) & mask;
    return (float)x * inv;
}

inline float Randf(float lo, float hi, uint32_t tid = 0u)
{
    return lo + Randf(tid) * (hi - lo);
}
