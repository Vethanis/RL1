#pragma once

#include "templates.h"

template<typename T, usize len>
static Slice<T> ToSlice(T(&arr)[len])
{
    return { arr, len };
}
template<typename T, usize len>
static Slice<const T> ToSlice(const T(&arr)[len])
{
    return { arr, len };
}
template<typename T, usize len>
static Slice<const T> ToCSlice(T(&arr)[len])
{
    return { arr, len };
}

template<typename T>
static Slice<T> ToSlice(T* ptr, usize len)
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}
template<typename T>
static Slice<const T> ToSlice(const T* ptr, usize len)
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}
template<typename T>
static Slice<const T> ToCSlice(T* ptr, usize len)
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}

template<typename T>
static Slice<const T> ToConst(Slice<T> x)
{
    return { x.begin(), x.size() };
}

template<typename T>
static Slice<T> Subslice(Slice<T> x, usize start, usize count)
{
    DebugAssert(start + count <= x.size());
    return { x.begin() + start, count };
}
template<typename T>
static Slice<const T> Subslice(Slice<const T> x, usize start, usize count)
{
    DebugAssert(start + count <= x.size());
    return { x.begin() + start, count };
}
template<typename T>
static Slice<const T> CSubslice(Slice<T> x, usize start, usize count)
{
    return Subslice(ToConst(x), start, count);
}

template<typename T>
static Slice<T> Opaque(Slice<T> x, usize idx, usize typeSize)
{
    return Subslice(x, idx * typeSize, typeSize);
}
template<typename T>
static Slice<const T> Opaque(Slice<const T> x, usize idx, usize typeSize)
{
    return Subslice(x, idx * typeSize, typeSize);
}
template<typename T>
static Slice<const T> COpaque(Slice<T> x, usize idx, usize typeSize)
{
    return CSubslice(x, idx * typeSize, typeSize);
}

template<typename To, typename From>
static Slice<To> ToType(Slice<From> x)
{
    return { (To*)x.begin(), x.bytes() / sizeof(To) };
}
template<typename To, typename From>
static Slice<const To> ToType(Slice<const From> x)
{
    return { (const To*)x.begin(), x.bytes() / sizeof(To) };
}
template<typename To, typename From>
static Slice<const To> ToCType(Slice<From> x)
{
    return { (const To*)x.begin(), x.bytes() / sizeof(To) };
}

template<typename B, typename A>
static Slice<B> ToType(A* ptr, usize len)
{
    return ToType<B>(ToSlice(ptr, len));
}
template<typename B, typename A>
static Slice<const B> ToCType(A* ptr, usize len)
{
    return ToType<const B>(ToCSlice(ptr, len));
}
template<typename B, typename A>
static Slice<const B> ToType(const A* ptr, usize len)
{
    return ToType<const B>(ToSlice(ptr, len));
}

// ----------------------------------------------------------------------------

template<typename T>
static ByteSlice ToBytes(Slice<T> x)
{
    return ToType<u8>(x);
}
template<typename T>
static CByteSlice ToBytes(Slice<const T> x)
{
    return ToType<const u8>(x);
}
template<typename T>
static CByteSlice ToCBytes(Slice<T> x)
{
    return ToCType<const u8>(x);
}

// ----------------------------------------------------------------------------

template<typename T>
static DwordSlice ToDwords(Slice<T> x)
{
    return ToType<u32>(x);
}
template<typename T>
static CDwordSlice ToDwords(Slice<const T> x)
{
    return ToType<const u32>(x);
}
template<typename T>
static CDwordSlice ToCDwords(Slice<T> x)
{
    return ToType<const u32>(x);
}

// ----------------------------------------------------------------------------

template<typename T>
static QwordSlice ToQwords(Slice<T> x)
{
    return ToType<u64>(x);
}
template<typename T>
static CQwordSlice ToQwords(Slice<const T> x)
{
    return ToType<const u64>(x);
}
template<typename T>
static CQwordSlice ToCQwords(Slice<T> x)
{
    return ToType<const u64>(x);
}

// ----------------------------------------------------------------------------

template<typename T>
static FloatSlice ToFloats(Slice<T> x)
{
    return ToType<f32>(x);
}
template<typename T>
static CFloatSlice ToFloats(Slice<const T> x)
{
    return ToType<const f32>(x);
}
template<typename T>
static CFloatSlice ToCFloats(Slice<T> x)
{
    return ToType<const f32>(x);
}

// ----------------------------------------------------------------------------
