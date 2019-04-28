#pragma once

#include "lang.h"
#include "memory.h"

template<u32 t_cap>
inline u32 circular_prev(u32 i)
{
    constexpr u32 mask = t_cap - 1u;
    return (i + mask) & mask;
}

template<u32 t_cap>
inline u32 circular_next(u32 i)
{
    constexpr u32 mask = t_cap - 1u;
    return (i + 1u) & mask;
}

template<typename T, u32 t_cap>
struct CircularStack
{
    T   m_data[t_cap];
    u32 m_head;

    inline u32 peek_idx() const
    {
        return circular_prev<t_cap>(m_head);
    }
    inline u32 stare_idx() const
    {
        return circular_prev<t_cap>(peek_idx());
    }
    inline void push(T x)
    {
        m_data[m_head] = x;
        m_head = circular_next<t_cap>(m_head);
    }
    inline T peek() const
    {
        return m_data[peek_idx()];
    }
    inline T stare() const
    {
        return m_data[stare_idx()];
    }
    inline void pop()
    {
        m_head = peek_idx();
        EraseR(m_data[m_head]);
    }

    inline void clear()
    {
        EraseR(*this);
    }

    inline       T& operator[](u32 i)       { return m_data[i]; }
    inline const T& operator[](u32 i) const { return m_data[i]; }
};
