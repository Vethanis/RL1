#pragma once

#include "array.h"
#include "find.h"
#include "remove.h"

template<typename K, typename V>
struct Dict1D
{
    Array<K>   m_keys;
    Array<V>   m_values;

    inline usize size() const
    {
        return m_keys.size();
    }
    inline i64 findKey(K key) const
    {
        return RFind(m_keys.cslice(), key, CompareV<K>);
    }
    inline i64 findValue(V value) const
    {
        return RFind(m_values.cslice(), value, CompareV<V>);
    }
    inline V* get(K key)
    {
        const i64 idx = findKey(key);
        if(idx == -1)
        {
            return nullptr;
        }
        return &m_values[idx];
    }
    inline const V* get(K key) const
    {
        const i64 idx = findKey(key);
        if(idx == -1)
        {
            return nullptr;
        }
        return &m_values[idx];
    }
    inline void addFast(K key, V value)
    {
        m_keys.grow()   = key;
        m_values.grow() = value;
    }
    inline bool add(K key, V value)
    {
        if(findKey(key) == -1)
        {
            m_keys.grow()   = key;
            m_values.grow() = value;
            return true;
        }
        return false;
    }
    inline void removeAtIdx(i64 idx)
    {
        m_keys.m_size   = SwapRemove(m_keys.slice(),   idx);
        m_values.m_size = SwapRemove(m_values.slice(), idx);
    }
    inline bool remove(K key)
    {
        const i64 idx = findKey(key);
        if(idx == -1)
        {
            return false;
        }
        removeAtIdx(idx);
        return true;
    }
    inline bool removeValue(V value)
    {
        const i64 idx = findValue(value);
        if(idx == -1)
        {
            return false;
        }
        removeAtIdx(idx);
        return true;
    }
    inline void clear()
    {
        m_keys.clear();
        m_values.clear();
    }
    inline void reset()
    {
        m_keys.reset();
        m_values.reset();
    }
};
