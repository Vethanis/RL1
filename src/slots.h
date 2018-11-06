#pragma once

#include "slot.h"

#include "blockalloc.h"
#include "array.h"
#include "macro.h"

template<typename T>
struct Slots
{
    struct Item
    {
        T*       ptr;
        uint32_t gen;
    };
    BlockAlloc<T>   m_storage;
    Array<Item>     m_items;

    void Add(slot s)
    {
        if(s.id >= m_items.count())
        {
            int32_t oldCount = m_items.count();
            m_items.resize(s.id + 1);
            for(int32_t i = oldCount; i < m_items.count(); ++i)
            {
                memset(&m_items[i], 0, sizeof(Item));
            }
        }
        Item& item = m_items[s.id];
        Assert(!item.ptr);
        item.gen = s.gen;
        item.ptr = m_storage.Alloc();
    }
    void Remove(slot s)
    {
        Assert(Exists(s));
        Item& item = m_items[s.id];
        Assert(item.ptr);
        m_storage.Free(item.ptr);
        item.ptr = nullptr;
        item.gen++;
    }
    inline T* Get(slot s)
    {
        Assert(Exists(s));
        return m_items[s.id].ptr;
    }
    inline bool Exists(slot s) const 
    {
        return s.id < m_items.count() && s.gen == m_items[s.id].gen;
    }
};
