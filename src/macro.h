#pragma once

#define MaxPathLen          size_t(256)
#define PageSize            size_t(4096)
#define MinAlign            size_t(16)
#define MaxAlign            size_t(4096)

#define CountOf(x)          ( sizeof(x) / (sizeof((x)[0])) )

#define WhenTrue(x, expr)   { if ( (x)) { expr; } }
#define WhenFalse(x, expr)  { if (!(x)) { expr; } }

#define StaticAssert(x)     typedef char __C_ASSERT__[(x) ? 1 : -1];
#define CountAssert(x, len) StaticAssert(CountOf(x) == len)
#define CountAssertEQ(x, y) StaticAssert(CountOf(x) == CountOf(y))

#define Interrupt()         __debugbreak()
#define Assert(x)           WhenFalse(x, Interrupt())

#if _DEBUG
    #define DebugOnly(x)    x
#else
    #define DebugOnly(x)
#endif // _DEBUG

#define DebugInterrupt()    DebugOnly(Interrupt())
#define DebugAssert(x)      DebugOnly(Assert(x))

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
