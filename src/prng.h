#pragma once

#include "scalar_types.h"

// *Really* minimal PCG32 code / (c) 2014 M.E. O'Neill / pcg-random.org
// Licensed under Apache License 2.0 (NO WARRANTY, etc. see website)

typedef struct { u64 state;  u64 inc; } pcg32_random_t;

u32 pcg32_random_r(pcg32_random_t* rng);

// -------------------------------------------------------------------
// Modifications below by Kara Hughes
// -------------------------------------------------------------------

enum 
{
    NumPCGStates = 16,
};

extern pcg32_random_t g_pcgstate[NumPCGStates];

inline void SRand(u64 value)
{
    for(u32 i = 0; i < NumPCGStates; ++i)
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
    constexpr u32 mask = NumPCGStates - 1u;
    pcg32_random_t* state = g_pcgstate + (tid & mask);
    return pcg32_random_r(state);
}

inline u32 Rand(u32 lo, u32 hi, u32 tid = 0u)
{
    return lo + (Rand(tid) % (hi - lo));
}

inline f32 Randf(u32 tid = 0u)
{
    // floats only have 24 bits of precision, thus 24 bit mask
    constexpr u32 mask = 0xFFFFFF;
    constexpr f32 inv = 1.0f / (f32)mask;
    u32 x = Rand(tid) & mask;
    return (f32)x * inv;
}

inline f32 Randf(f32 lo, f32 hi, u32 tid = 0u)
{
    return lo + Randf(tid) * (hi - lo);
}
