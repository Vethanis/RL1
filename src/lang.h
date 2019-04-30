#pragma once

#include <stddef.h>
#include <stdint.h>
#include <signal.h>

#define let                     const auto
#define letmut                  auto
#define def                     static constexpr

#define LEN(x)                  ( sizeof(x) / (sizeof((x)[0])) )
#define Format(dst, fmt, ...)   { snprintf(dst, LEN(dst) - 1, fmt, __VA_ARGS__); dst[LEN(dst) - 1] = 0; }

#if _MSVC_VER
    #define MsvcOnly(x)     x
    #define PosixOnly(x)
#else
    #define MsvcOnly(x)
    #define PosixOnly(x)    x
#endif // _MSVC_VER

#if _DEBUG
    #define DebugOnly(x)    x
    #define ReleaseOnly(x)
#else
    #define DebugOnly(x)
    #define ReleaseOnly(x)  x
#endif // _DEBUG

#define WhenTrue(x, expr)   { if ( (x)) { expr; } }
#define WhenFalse(x, expr)  { if (!(x)) { expr; } }

#define StaticAssert(x)     typedef char __C_ASSERT__[(x) ? 1 : -1];
#define CountAssert(x, len) StaticAssert(LEN(x) == len)
#define CountAssertEQ(x, y) StaticAssert(LEN(x) == LEN(y))

#define Interrupt()         raise(SIGINT)
#define Assert(x)           WhenFalse(x, Interrupt())
#define Verify(x)           Assert(x)
#define VerifyEQ(lhs, rhs)  Assert((lhs) == (rhs))

#define DebugInterrupt()    DebugOnly(Interrupt())
#define DebugAssert(x)      DebugOnly(Assert(x))

using usize = size_t;
using isize = ptrdiff_t;

using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;

using f32 = float;
using f64 = double;

using cstr = const char*;

struct Slot
{
    u32 id;
    u32 gen;
};

def usize MaxPathLen = 256;
def usize PageSize = 4096;
def usize MinAlign = 16;
def usize MaxAlign = PageSize;

template<typename T>
inline constexpr T Min(T a, T b)
{
    return a < b ? a : b;
}

template<typename T>
inline constexpr  T Max(T a, T b)
{
    return a > b ? a : b;
}

template<typename T>
inline constexpr  T Clamp(T x, T lo, T hi)
{
    return Min(hi, Max(lo, x));
}

template<typename T>
inline constexpr T Select(T x0, T x1, bool s)
{
    return s ? x1 : x0;
}

template<typename T>
inline constexpr T Lerp(T a, T b, T i)
{
    return a + i * (b - a);
}

template<typename T>
inline constexpr T DivCeil(T nom, T denom)
{
    return (nom + (denom - T(1))) / denom;
}

template<typename T>
inline constexpr T AlignGrow(T x, T align)
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

template<typename T>
inline constexpr T Sum(const T* x, usize len)
{
    T y = T(0);
    for (usize i = 0; i < len; ++i)
    {
        y += x[i];
    }
    return y;
}

template<typename T>
inline constexpr T MaxA(const T* x, usize len)
{
    usize y = 0;
    for(usize i = 0; i < len; ++i)
    {
        if(x[i] > x[y])
        {
            y = i;
        }
    }
    return x[y];
}

template<typename T>
inline constexpr T MinA(const T* x, usize len)
{
    usize y = 0;
    for(usize i = 0; i < len; ++i)
    {
        if(x[i] < x[y])
        {
            y = i;
        }
    }
    return x[y];
}

template<typename T>
inline void IndexFill(T* x, T len)
{
    for(T i = T(0); i < len; ++i)
    {
        x[i] = i;
    }
}
