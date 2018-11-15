#pragma once

#include "slot.h"
#include "array.h"

template<typename T>
struct gen_array
{
    Array<T>        m_data;
    Array<uint32_t> m_gen;
    Array<uint32_t> m_free;

    slot Create()
    {
        if(m_free.empty())
        {
            m_free.grow() = (uint32_t)m_data.count();
            m_data.grow();
            m_gen.grow() = 0;
        }

        slot s;
        s.id = m_free.back();
        m_free.pop();
        s.gen = m_gen[s.id];

        memset(&m_data[s.id], 0, sizeof(T));

        return s;
    }
    inline T* Get(slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        return &GetUnchecked(s);
    }
    inline const T* Get(slot s) const
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        return &GetUnchecked(s);
    }
    // for when you check Exists() first and layer behavior above this
    inline T& GetUnchecked(slot s)
    {
        return m_data[s.id];
    }
    inline const T& GetUnchecked(slot s) const
    {
        return m_data[s.id];
    }
    inline void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        DestroyUnchecked(s);
    }
    // for when you check Exists() first and layer behavior above this
    inline void DestroyUnchecked(slot s)
    {
        m_gen[s.id]++;
        m_free.grow() = s.id;
    }
    inline bool Exists(slot s) const
    {
        return s.id < (uint32_t)m_data.count() && m_gen[s.id] == s.gen; 
    }
};
