#pragma once

#include "lang.h"
#include "array.h"

template<typename K, typename V>
struct Dict1D
{
    Array<K> m_keys;
    Array<V> m_values;

    inline usize size() const
    {
        return m_keys.size();
    }
    inline isize findKey(K key) const
    {
        return m_keys.rfind(key);
    }
    inline isize findValue(V value) const
    {
        return m_values.rfind(value);
    }
    inline V* get(K key)
    {
        const isize idx = findKey(key);
        if(idx == -1)
        {
            return nullptr;
        }
        return &m_values[idx];
    }
    inline const V* get(K key) const
    {
        const isize idx = findKey(key);
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
    inline void removeAtIdx(isize idx)
    {
        m_keys.remove(idx);
        m_values.remove(idx);
    }
    inline bool remove(K key)
    {
        const isize idx = findKey(key);
        if(idx == -1)
        {
            return false;
        }
        removeAtIdx(idx);
        return true;
    }
    inline bool removeValue(V value)
    {
        const isize idx = findValue(value);
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
