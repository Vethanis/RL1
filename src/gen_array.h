#pragma once

#include "lang.h"
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

        let idx = m_free.pop_value();
        let gen = (m_gen[idx] |= 1u);

        return { idx, gen };
    }
    inline bool destroy(Slot s)
    {
        if(!exists(s))
        {
            return false;
        }

        m_free.grow() = s.id;
        m_gen[s.id] = (m_gen[s.id] + 2u) & (~0x1);

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
    // gens[id] & 1 => IsAlive(id)
    inline Slice<const u32> gens() const
    {
        return m_gen.cslice();
    }
    inline usize capacity()
    {
        return m_gen.size();
    }
};
