#pragma once

#include "slice.h"

using Allocation = Slice<u8>;
using AllocFn    = Allocation (*)(size_t bytes, size_t align);
using ReallocFn  = Allocation (*)(Allocation prev, size_t bytes, size_t align);
using FreeFn     =       void (*)(Allocation x);

struct Allocator
{
    const AllocFn   m_alloc;
    const ReallocFn m_realloc;
    const FreeFn    m_free;
    const size_t    m_align;

    template<typename T>
    inline Slice<T> Alloc(size_t reqLen) const
    {
        return ToType<T>(m_alloc(reqLen * sizeof(T), m_align));
    }

    template<typename T>
    inline void Realloc(Slice<T>& mem, size_t reqLen) const
    {
        const size_t prevLen = mem.size();
        if(reqLen > prevLen)
        {
            const size_t newBytes = Max(reqLen, prevLen << size_t(1)) * sizeof(T);
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
