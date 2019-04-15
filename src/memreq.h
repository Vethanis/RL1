#pragma once

#include "macro.h"

struct MemReq
{
    size_t bytes;
    size_t align;
};

inline MemReq FixRequest(size_t reqBytes, size_t reqAlign)
{
    const size_t newAlign = Clamp(reqAlign, MinAlign, MaxAlign);
    const size_t newBytes = AlignGrow(reqBytes, newAlign);
    return { newBytes, newAlign };
};
