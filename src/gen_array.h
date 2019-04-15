#pragma once

#include "slot.h"
#include "array.h"

template<const Allocator& t_alloc>
struct GenIndices
{
    Array<u32, t_alloc> m_gen;
    Array<u32, t_alloc> m_free;

    inline bool exists(Slot s) const
    {
        const u32 idx = s.id;
        if(idx >= m_gen.size())
        {
            return false;
        }
        const u32 gen = m_gen[idx];
        return (gen & 1u) && (gen == s.gen);
    }
    inline Slot create()
    {
        if(m_free.empty())
        {
            m_free.grow() = m_gen.size();
            m_gen.grow()  = 0u;
        }

        const u32 idx = m_free.back();
        m_free.pop();

        const u32 gen = m_gen[idx] | 1u;
        m_gen[idx] = gen;

        Slot s;
        s.id  = idx;
        s.gen = gen;

        return s;
    }
    inline bool destroy(Slot s)
    {
        if(!exists(s))
        {
            return false;
        }

        const u32 idx = s.id;
        m_free.grow() = idx;
        m_gen[idx] = ( (m_gen[idx] >> 1u) + 1u ) << 1u;

        return true;
    }
    inline void reset()
    {
        m_gen.reset();
        m_free.reset();
    }
    inline void clear()
    {
        m_gen.clear();
        m_free.clear();
    }
    inline Slice<const u32> gens() const
    {
        return m_gen.cslice();
    }
    inline size_t capacity()
    {
        return m_gen.size();
    }
};
