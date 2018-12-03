#pragma once

#include "array.h"

template<typename K, typename V, uint64_t width>
struct Dict
{
    Array<K>            m_keys[width];
    Array<V>            m_data[width];
    uint32_t            m_count = 0u;

    inline void Insert(K key, const V& item)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        if(idx == -1)
        {
            m_data[slot].grow() = item;
            m_keys[slot].grow() = key;
            ++m_count;
        }
    }
    inline V* Get(K key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline const V* Get(K key) const
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline void Remove(K key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        if(idx != -1)
        {
            m_data[slot].remove(idx);
            m_keys[slot].remove(idx);
            --m_count;
        }
    }
    inline uint32_t Count() const 
    {
        return m_count;
    }
};
