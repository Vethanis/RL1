#pragma once

#include "slot.h"

#include "array.h"
#include "macro.h"

template<typename T>
struct Slots
{
    Array2<T, slot> m_data;
    Array<int32_t>  m_s2d;

    void Add(slot s)
    {
        if(s.id >= m_s2d.count())
        {
            int32_t oldCount = m_s2d.count();
            m_s2d.resize(s.id + 1);
            for(int32_t i = oldCount; i < m_s2d.count(); ++i)
            {
                m_s2d[i] = -1;
            }
        }
        int32_t& idx = m_s2d[s.id];
        if(idx == -1)
        {
            idx = m_data.count();
            m_data.grow();
            m_data.GetU() = s;
            T& item = m_data.GetT();
            memset(&item, 0, sizeof(T));
        }
        else
        {
            Assert(false);
        }
    }
    void Remove(slot s)
    {
        if(s.id >= m_s2d.count())
        {
            return;
        }
        int32_t idx = m_s2d[s.id];
        if(idx != -1)
        {
            Assert(idx < m_data.count());
            slot t = m_data.GetU(idx);
            Assert(t.id == s.id);
            if(t.gen != s.gen)
            {
                return;
            }
            slot r = m_data.backU();
            m_s2d[r.id] = idx;
            m_data.remove(idx);
            m_s2d[s.id] = -1;
        }
        else
        {
            Assert(false);
        }
    }
    T* Get(slot s)
    {
        if(s.id >= m_s2d.count())
        {
            return nullptr;
        }
        int32_t idx = m_s2d[s.id];
        if(idx == -1)
        {
            return nullptr;
        }
        Assert(idx < m_data.count());
        slot t = m_data.GetU(idx);
        if(t.gen != s.gen)
        {
            return nullptr;
        }
        Assert(t.id == s.id);
        return &m_data.GetT(idx);
    }
    inline T& Get(int32_t i)
    {
        return m_data.GetT(i);
    }
    inline const T& Get(int32_t i) const
    {
        return m_data.GetT(i);
    }
    inline slot GetSlot(int32_t i) const 
    {
        return m_data.GetU(i);
    }
    inline int32_t Count() const { return m_data.count(); }
};