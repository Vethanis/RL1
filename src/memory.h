#pragma once

#include <string.h>
#include "macro.h"

template<typename T>
inline void EraseR(T& x)
{
    memset(&x, 0, sizeof(T));
}

template<typename T, usize capacity>
inline void EraseA(T (&x)[capacity])
{
    memset(x, 0, sizeof(T) * capacity);
}

template<typename T>
inline void EraseP(T* x, usize count)
{
    memset(x, 0, sizeof(T) * count);
}

template<typename T>
inline void CopyR(T& dst, const T& src)
{
    memcpy(&dst, &src, sizeof(T));
}

template<typename T>
inline void CopyP(T* dst, const T* src, usize count)
{
    memcpy(dst, src, sizeof(T) * count);
}

template<typename T>
inline void MoveR(T& dst, const T& src)
{
    memmove(&dst, &src, sizeof(T));
}

template<typename T>
inline void MoveP(T* dst, const T* src, usize count)
{
    memmove(dst, src, sizeof(T) * count);
}

template<typename T>
inline void Swap(T& a, T& b)
{
    u8 mem[sizeof(T)];
    memcpy(mem, &a, sizeof(T));
    memcpy(&a, &b, sizeof(T));
    memcpy(&b, mem, sizeof(T));
}

template<typename T>
inline i32 CompareR(const T& a, const T& b)
{
    return memcmp(&a, &b, sizeof(T));
}

template<typename T>
inline i32 CompareP(const T* a, const T* b, usize len)
{
    return memcmp(a, b, sizeof(T) * len);
}

template<typename T>
inline i32 CompareV(T a, T b)
{
    return memcmp(&a, &b, sizeof(T));
}
