#pragma once

#include "array.h"

template<typename T, uint64_t width>
struct Dict
{
    Array<T>        m_data[width];
    Array<uint64_t> m_keys[width];

    inline void Insert(uint64_t key, const T& item)
    {
        uint64_t slot = key % width;
        m_data[slot].grow() = item;
        m_keys[slot].grow() = key;
    }
    inline T* Get(uint64_t key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline const T* Get(uint64_t key) const
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        return idx == -1 ? nullptr : &(m_data[slot][idx]);
    }
    inline void Remove(uint64_t key)
    {
        uint64_t slot = key % width;
        int32_t idx = m_keys[slot].find(key);
        if(idx != -1)
        {
            m_data[slot].remove(idx);
            m_keys[slot].remove(idx);
        }
    }
};
