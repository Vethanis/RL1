#pragma once

#include <stddef.h>
#include <stdint.h>

#define let         const auto
#define letmut      auto
#define def         static constexpr
#define CountOf(x)  ( sizeof(x) / (sizeof((x)[0])) )

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
#define CountAssert(x, len) StaticAssert(CountOf(x) == len)
#define CountAssertEQ(x, y) StaticAssert(CountOf(x) == CountOf(y))

#define Interrupt()         MsvcOnly(__debugbreak()) PosixOnly(__builtin_trap())
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

template<typename T>
struct Slice
{
    T*    m_ptr;
    usize m_len;

    inline usize    size()  const { return m_len; }
    inline T*       begin()       { return m_ptr; }
    inline const T* begin() const { return m_ptr; }

    inline usize    bytes() const { return m_len * sizeof(T); }
    inline bool     empty() const { return m_len == (usize)0; }
    inline T*       end()         { return m_ptr + m_len; }
    inline const T* end()   const { return m_ptr + m_len; }

    inline T& operator[](usize i)
    {
        DebugAssert(i < m_len);
        return m_ptr[i];
    }
    inline const T& operator[](usize i) const
    {
        DebugAssert(i < m_len);
        return m_ptr[i];
    }
};

using ByteSlice = Slice<u8>;
using CByteSlice = Slice<const u8>;

using DwordSlice = Slice<u32>;
using CDwordSlice = Slice<const u32>;

using QwordSlice = Slice<u64>;
using CQwordSlice = Slice<const u64>;

using FloatSlice = Slice<f32>;
using CFloatSlice = Slice<const f32>;
