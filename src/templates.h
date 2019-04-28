#pragma once

#include "lang.h"

def usize MaxPathLen = 256;
def usize PageSize = 4096;
def usize MinAlign = 16;
def usize MaxAlign = PageSize;

template<typename T, typename Y>
inline T* Cast(Slice<Y> x)
{
    return reinterpret_cast<T*>(x.m_ptr);
}

template<typename T, typename Y>
inline const T* Cast(Slice<const Y> x)
{
    return reinterpret_cast<const T*>(x.m_ptr);
}

template<typename T>
static constexpr T Min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T>
static constexpr  T Max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T>
static constexpr  T Clamp(T x, T lo, T hi)
{
    return Min(hi, Max(lo, x));
}

template<typename T>
static constexpr T Select(T x0, T x1, bool s)
{
    return s ? x1 : x0;
}

template<typename T>
static constexpr T Lerp(T a, T b, T i)
{
    return a + i * (b - a);
}

template<typename T>
static constexpr T DivCeil(T nom, T denom)
{
    return (nom + (denom - T(1))) / denom;
}

template<typename T>
static constexpr T AlignGrow(T x, T align)
{
    return DivCeil(x, align) * align;
}

inline void SetFlag(u32& x, u32 y)
{
    x |= u32(1) << y;
}
inline void UnsetFlag(u32& x, u32 y)
{
    x &= ~(u32(1) << y);
}
inline bool HasFlag(u32 x, u32 y)
{
    return (x >> y) & u32(1);
}
inline bool HasMask(u32 x, u32 mask)
{
    return (x & mask) == mask;
}
