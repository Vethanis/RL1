#pragma once

#include "macro.h"
#include "gen_array.h"
#include "dict.h"
#include "fnv.h"
#include "hashstring.h"

template<typename T, uint64_t width, void (*loadFn)(T*, Hash) = nullptr, void (*destroyFn)(T*) = nullptr>
struct Store
{
    struct Item
    {
        T t;
        Hash hash;
        int32_t refcount;
    };
    gen_array<Item>             m_items;
    Dict<Hash, slot, width>     m_dict;

    inline slot Create(const char* name)
    {
        return Create(Hash(name));
    }
    slot Create(Hash hash)
    {
        slot s = Find(hash);
        if(Exists(s))
        {
            Item& item = m_items.GetUnchecked(s);
            item.refcount++;
            return s;
        }
        s = m_items.Create();
        Item& item = m_items.GetUnchecked(s);
        if(loadFn)
        {
            loadFn(&item.t, hash);
        }
        item.hash = hash;
        item.refcount = 1;
        m_dict.Insert(hash, s);
        return s;
    }
    inline slot Create(const char* name, const T& t)
    {
        return Create(Hash(name), t);
    }
    slot Create(Hash hash, const T& t)
    {
        slot s = Find(hash);
        if(Exists(s))
        {
            return slot();
        }
        s = m_items.Create();
        Item& item = m_items.GetUnchecked(s);
        item.t = t;
        item.hash = hash;
        item.refcount = 1;
        m_dict.Insert(hash, s);
        return s;
    }
    inline void IncRef(slot s)
    {
        if(Exists(s))
        {
            m_items.GetUnchecked(s).refcount++;
        }
    }
    inline void DecRef(slot s)
    {
        Destroy(s);
    }
    inline int32_t RefCount(slot s) const
    {
        if(Exists(s))
        {
            return m_items.GetUnchecked(s).refcount;
        }
        return 0;
    }
    inline void Destroy(const char* name)
    {
        Destroy(Hash(name));
    }
    inline void Destroy(Hash hash)
    {
        slot s = Find(hash);
        Destroy(s);
    }
    inline void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        Item& item = m_items.GetUnchecked(s);
        item.refcount--;
        if(item.refcount == 0)
        {
            DestroyUnchecked(s);
        }
    }
    inline void DestroyUnchecked(slot s)
    {
        Item& item = m_items.GetUnchecked(s);
        if(destroyFn)
        {
            destroyFn(&item.t);
        }
        m_dict.Remove(item.hash);
        m_items.DestroyUnchecked(s);
    }
    inline bool Exists(slot s) const
    {
        return m_items.Exists(s);
    }
    inline bool Exists(const char* name) const 
    {
        return Exists(Find(name));
    }
    inline bool Exists(Hash hash) const 
    {
        return Exists(Find(hash));
    }
    inline slot Find(Hash hash) const
    {
        const slot* s = m_dict.Get(hash);
        return s ? *s : slot();
    }
    inline slot Find(const char* name) const 
    {
        return Find(Hash(name));
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
