#pragma once

#include "macro.h"
#include "memory.h"

template<u32 t_cap>
static u32 circular_prev(u32 i)
{
    constexpr u32 mask = t_cap - 1u;
    return (i + mask) & mask;
}

template<u32 t_cap>
static u32 circular_next(u32 i)
{
    constexpr u32 mask = t_cap - 1u;
    return (i + 1u) & mask;
}

template<typename T, u32 t_cap>
struct CircularStack
{
    T   m_data[t_cap];
    u32 m_head;

    inline void push(T x)
    {
        m_data[m_head] = x;
        m_head = circular_next<t_cap>(m_head);
    }
    inline T peek() const
    {
        return m_data[circular_prev<t_cap>(m_head)];
    }
    inline void pop()
    {
        m_head = circular_prev<t_cap>(m_head);
        EraseR(m_data[m_head]);
    }

    inline void clear()
    {
        EraseR(*this);
    }

    inline       T& operator[](u32 i)       { return m_data[i]; }
    inline const T& operator[](u32 i) const { return m_data[i]; }

    inline u32 top_idx()       const { return circular_prev<t_cap>(m_head); }
    inline u32 bottom_idx()    const { return m_head;                       }

    inline i32 find(T key) const
    {
        const T*  ptr = m_data;
        const u32 bot = bottom_idx();

        for(u32 i = top_idx();
            i != bot;
            i = circular_prev<t_cap>(i))
        {
            if(key == ptr[i])
            {
                return (i32)i;
            }
        }
        return -1;
    }
};
