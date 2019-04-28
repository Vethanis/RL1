#pragma once

#include "lang.h"

struct MemReq
{
    usize bytes;
    usize align;
};

inline MemReq FixRequest(usize reqBytes, usize reqAlign)
{
    let newAlign = Clamp(reqAlign, MinAlign, MaxAlign);
    let newBytes = AlignGrow(reqBytes, newAlign);
    return { newBytes, newAlign };
};
