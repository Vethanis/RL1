#pragma once

#include <stdint.h>
#include <new>

#include "array.h"
#include "macro.h"

struct slot
{
    int32_t id;
    int32_t gen;
};

template<typename T, bool POD = true>
struct Slots
{
    Array<T> m_data;
    Array<int32_t> m_gen;
    Array<int32_t> m_live;
    Array<int32_t> m_free;

    Slots()
    {

    }
    ~Slots()
    {
        Clear();
    }
    Slots(const Slots& other)
    {
        Assert(false);
    }
    Slots(Slots&& other) noexcept
    {
        Assert(false);
    }
    Slots& operator=(const Slots& other)
    {
        Assert(false);
        return *this;
    }
    Slots& operator=(Slots&& other) noexcept
    {
        Assert(false);
        return *this;
    }
    void Clear()
    {
        if(!POD)
        {
            for(int32_t idx : m_live)
            {
                T* item = &m_data[idx];
                item->~T();
            }
        }
        m_live.clear();
    }
    void Reset()
    {
        Clear();
        m_data.reset();
        m_gen.reset();
        m_live.reset();
        m_free.reset();
    }
    slot Add()
    {
        slot s;

        if(!m_free.empty())
        {
            s.id = m_free.back();
            m_free.pop();
        }
        else
        {
            s.id = m_data.count();
            m_data.grow();
            m_gen.grow() = 0;
        }

        m_live.grow() = s.id;
        s.gen = m_gen[s.id];

        if(!POD)
        {
            T* item = &m_data[s.id];
            new (item) T();
        }
        
        return s;
    }
    void Remove(slot s)
    {
        if(s.gen != m_gen[s.id])
        {
            return;
        }

        m_gen[s.id]++;
        m_free.grow() = s.id;

        if(!POD)
        {
            T* item = &m_data[s.id];
            item->~T();
        }

        m_live.findRemove(s.id);
    }
    inline T* operator[](slot s)
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data[s.id];
    }
    inline const T* operator[](slot s) const
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data[s.id];
    }
    inline int32_t Count() const 
    {
        return m_live.count();
    }
    inline T& operator[](int32_t i)
    {
        int32_t idx = m_live[i];
        return m_data[idx];
    }
    inline const T& operator[](int32_t i) const
    {
        int32_t idx = m_live[i];
        return m_data[idx];
    }
};


template<typename T, typename U, bool POD = true>
struct Slots2
{
    Array2<T, U>    m_data;
    Array<int32_t>  m_gen;
    Array<int32_t>  m_live;
    Array<int32_t>  m_free;

    Slots2()
    {

    }
    ~Slots2()
    {
        Clear();
    }
    Slots2(const Slots2& other)
    {
        Assert(false);
    }
    Slots2(Slots2&& other) noexcept
    {
        Assert(false);
    }
    Slots2& operator=(const Slots2& other)
    {
        Assert(false);
        return *this;
    }
    Slots2& operator=(Slots2&& other) noexcept
    {
        Assert(false);
        return *this;
    }
    void Clear()
    {
        if(!POD)
        {
            for(int32_t idx : m_live)
            {
                T* t = &m_data.m_ts[idx];
                U* u = &m_data.m_us[idx];
                t->~T();
                u->~U();
            }
        }
        m_live.clear();
    }
    void Reset()
    {
        Clear();
        m_data.reset();
        m_gen.reset();
        m_live.reset();
        m_free.reset();
    }
    slot Add()
    {
        slot s;

        if(!m_free.empty())
        {
            s.id = m_free.back();
            m_free.pop();
        }
        else
        {
            s.id = m_data.count();
            m_data.grow();
            m_gen.grow() = 0;
        }

        m_live.grow() = s.id;
        s.gen = m_gen[s.id];

        if(!POD)
        {
            T* t = &m_data.m_ts[s.id];
            U* u = &m_data.m_us[s.id];
            new (t) T();
            new (u) U();
        }
        
        return s;
    }
    void Remove(slot s)
    {
        if(s.gen != m_gen[s.id])
        {
            return;
        }

        m_gen[s.id]++;
        m_free.grow() = s.id;

        if(!POD)
        {
            T* t = &m_data.m_ts[s.id];
            U* u = &m_data.m_us[s.id];
            t->~T();
            u->~U();
        }

        m_live.findRemove(s.id);
    }
    inline T* GetT(slot s)
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data.m_ts[s.id];
    }
    inline const T* GetT(slot s) const
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data.m_ts[s.id];
    }
    inline U* GetU(slot s)
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data.m_us[s.id];
    }
    inline const U* GetU(slot s) const
    {
        if(s.gen != m_gen[s.id])
        {
            return nullptr;
        }
        return &m_data.m_us[s.id];
    }
    inline int32_t Count() const 
    {
        return m_live.count();
    }
    inline T& GetT(int32_t i)
    {
        int32_t idx = m_live[i];
        return m_data.m_ts[idx];
    }
    inline const T& GetT(int32_t i) const
    {
        int32_t idx = m_live[i];
        return m_data.m_ts[idx];
    }
    inline U& GetU(int32_t i)
    {
        int32_t idx = m_live[i];
        return m_data.m_us[idx];
    }
    inline const U& GetU(int32_t i) const
    {
        int32_t idx = m_live[i];
        return m_data.m_us[idx];
    }
};
