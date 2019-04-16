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
        return s.id < m_gen.size() && s.gen == m_gen[s.id];
    }
    inline Slot create()
    {
        if(m_free.empty())
        {
            m_free.grow() = (u32)m_gen.size();
            m_gen.grow()  = 0u;
        }

        const u32 idx = m_free.back();
        m_free.pop();

        return { idx, m_gen[idx] };
    }
    inline bool destroy(Slot s)
    {
        if(!exists(s))
        {
            return false;
        }

        m_free.grow() = s.id;
        m_gen[s.id]++;

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
