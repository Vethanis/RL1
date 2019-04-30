#pragma once

#include "lang.h"
#include "slice.h"

using Allocation = ByteSlice;
using AllocFn    = Allocation (*)(usize bytes, usize align);
using ReallocFn  = Allocation (*)(Allocation prev, usize bytes, usize align);
using FreeFn     =       void (*)(Allocation x);

struct Allocator
{
    const AllocFn   m_alloc;
    const ReallocFn m_realloc;
    const FreeFn    m_free;
    const usize    m_align;

    template<typename T>
    inline Slice<T> Alloc(usize reqLen) const
    {
        return ToType<T>(m_alloc(reqLen * sizeof(T), m_align));
    }

    template<typename T>
    inline void Realloc(Slice<T>& mem, usize reqLen) const
    {
        const usize prevLen = mem.size();
        if(reqLen > prevLen)
        {
            const usize newBytes = Max(reqLen, prevLen << usize(1)) * sizeof(T);
            mem = ToType<T>(m_realloc(ToBytes(mem), newBytes, m_align));
        }
    }

    template<typename T>
    inline void Free(Slice<T>& mem) const
    {
        m_free(ToBytes(mem));
        EraseR(mem);
    }
};
