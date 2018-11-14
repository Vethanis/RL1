#pragma once

#include "macro.h"
#include "gen_array.h"
#include "dict.h"
#include "fnv.h"

template<typename T, uint64_t width>
struct Store
{
    struct Item
    {
        T t;
        uint64_t hash;
    };
    gen_array<Item>     m_items;
    Dict<slot, width>   m_dict;

    slot Create(const char* name)
    {
        uint64_t hash = Fnv64(name);
        Assert(!Exists(hash));
        slot s = m_items.Create();
        m_items.GetUnchecked(s).hash = hash;
        m_dict.Insert(hash, s);
        return s;
    }
    inline void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        DestroyUnchecked(s);
    }
    inline void DestroyUnchecked(slot s)
    {
        Item& item = m_items.GetUnchecked(s);
        m_dict.Remove(item.hash);
        m_items.DestroyUnchecked(s);
    }
    inline bool Exists(slot s) const
    {
        return m_items.Exists(s);
    }
    inline bool Exists(const char* name) const 
    {
        return Find(name) != slot::CreateInvalid();
    }
    inline bool Exists(uint64_t hash) const 
    {
        return Find(hash) != slot::CreateInvalid();
    }
    inline slot Find(uint64_t hash) const
    {
        const slot* s = m_dict.Get(hash);
        return s ? *s : slot::CreateInvalid();
    }
    inline slot Find(const char* name) const 
    {
        return Find(Fnv64(name));
    }
    inline T* Get(slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Item& item = m_items.GetUnchecked(s);
        return &item.t;
    }
    inline const T* Get(slot s) const
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        const Item& item = m_items.GetUnchecked(s);
        return &item.t;
    }
    inline T& GetUnchecked(slot s)
    {
        return m_items.GetUnchecked(s).t;
    }
    inline const T& GetUnchecked(slot s) const 
    {
        return m_items.GetUnchecked(s).t;
    }
};
