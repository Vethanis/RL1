#pragma once

#include <stddef.h>
#include "macro.h"
#include "scalar_types.h"

template<typename T>
struct Slice
{
    T*      m_ptr;
    size_t  m_len;

    inline size_t   size()  const { return m_len; }
    inline T*       begin()       { return m_ptr; }
    inline const T* begin() const { return m_ptr; }

    inline size_t   bytes() const { return m_len * sizeof(T);  }
    inline bool     empty() const { return m_len == (size_t)0; }
    inline T*       end()         { return m_ptr + m_len;    }
    inline const T* end()   const { return m_ptr + m_len;    }

    inline T& operator[](size_t i) 
    { 
        DebugAssert(i < m_len); 
        return m_ptr[i];
    }
    inline const T& operator[](size_t i) const 
    { 
        DebugAssert(i < m_len); 
        return m_ptr[i];
    }

    inline Slice<const T> to_const() const 
    { 
        return { m_ptr, m_len };
    }

    template<typename U>
    inline Slice<U> cast() 
    {
        return { (U*)m_ptr, bytes() / sizeof(U) };
    }

    template<typename U>
    inline Slice<const U> cast() const 
    {
        return { (const U*)m_ptr, bytes() / sizeof(U) };
    }

    inline Slice<T> subslice(size_t start, size_t count) 
    {
        DebugAssert(start + count < m_len);
        return { m_ptr + start, count };
    }
    inline Slice<const T> subslice(size_t start, size_t count) const 
    {
        DebugAssert(start + count < m_len);
        return { m_ptr + start, count };
    }

    inline Slice<T> opaque(size_t idx, size_t typeSize)
    {
        return subslice(idx * typeSize, typeSize);
    }
    inline Slice<const T> opaque(size_t idx, size_t typeSize) const
    {
        return subslice(idx * typeSize, typeSize);
    }
};

// ----------------------------------------------------------------------------

using ByteSlice  = Slice<u8>;
using CByteSlice = Slice<const u8>;

using DwordSlice  = Slice<u32>;
using CDwordSlice = Slice<const u32>;

using QwordSlice  = Slice<u64>;
using CQwordSlice = Slice<const u64>;

using FloatSlice  = Slice<f32>;
using CFloatSlice = Slice<const f32>;

// ----------------------------------------------------------------------------

template<typename T, size_t len>
inline Slice<T> ToSlice(T (&arr)[len]) 
{
    DebugAssert(!arr ? !len : true);
    
    return { arr, len };
}
template<typename T, size_t len>
inline Slice<const T> ToCSlice(T (&arr)[len]) 
{
    DebugAssert(!arr ? !len : true);
    
    return { arr, len };
}
template<typename T, size_t len>
inline Slice<const T> ToSlice(const T (&arr)[len])
{
    DebugAssert(!arr ? !len : true);
    
    return { arr, len };
}

// ----------------------------------------------------------------------------

template<typename T>
inline Slice<T> ToSlice(T* ptr, size_t len) 
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}
template<typename T>
inline Slice<const T> ToCSlice(T* ptr, size_t len) 
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}
template<typename T>
inline Slice<const T> ToSlice(const T* ptr, size_t len) 
{
    DebugAssert(!ptr ? !len : true);

    return { ptr, len };
}

// ----------------------------------------------------------------------------

template<typename T>
inline ByteSlice ToBytes(Slice<T> x) 
{
    return x.cast<u8>();
}
template<typename T>
inline CByteSlice ToCBytes(Slice<T> x) 
{
    return x.cast<const u8>();
}
template<typename T>
inline CByteSlice ToBytes(Slice<const T> x) 
{
    return x.cast<const u8>();
}

// ----------------------------------------------------------------------------

template<typename T>
inline DwordSlice ToDwords(Slice<T> x) 
{
    return x.cast<u32>();
}
template<typename T>
inline CDwordSlice ToCDwords(Slice<T> x) 
{
    return x.cast<const u32>();
}
template<typename T>
inline CDwordSlice ToDwords(Slice<const T> x) 
{
    return x.cast<const u32>();
}

// ----------------------------------------------------------------------------

template<typename T>
inline QwordSlice ToQwords(Slice<T> x) 
{
    return x.cast<u64>();
}
template<typename T>
inline CQwordSlice ToCQwords(Slice<T> x) 
{
    return x.cast<const u64>();
}
template<typename T>
inline CQwordSlice ToQwords(Slice<const T> x) 
{
    return x.cast<const u64>();
}

// ----------------------------------------------------------------------------

template<typename T>
inline FloatSlice ToFloats(Slice<T> x) 
{
    return x.cast<f32>();
}
template<typename T>
inline CFloatSlice ToCFloats(Slice<T> x) 
{
    return x.cast<const f32>();
}
template<typename T>
inline CFloatSlice ToFloats(Slice<const T> x) 
{
    return x.cast<const f32>();
}

// ----------------------------------------------------------------------------

template<typename B, typename A>
inline Slice<B> ToType(Slice<A> x) 
{
    return x.cast<B>();
}
template<typename B, typename A>
inline Slice<const B> ToCType(Slice<A> x) 
{
    return x.cast<const B>();
}
template<typename B, typename A>
inline Slice<const B> ToType(Slice<const A> x) 
{
    return x.cast<const B>();
}

// ----------------------------------------------------------------------------

template<typename B, typename A>
inline Slice<B> ToType(A* ptr, size_t len)
{
    return ToType<B>(ToSlice(ptr, len));
}
template<typename B, typename A>
inline Slice<const B> ToCType(A* ptr, size_t len)
{
    return ToType<const B>(ToCSlice(ptr, len));
}
template<typename B, typename A>
inline Slice<const B> ToType(const A* ptr, size_t len)
{
    return ToType<const B>(ToSlice(ptr, len));
}

// ----------------------------------------------------------------------------
