#pragma once

#include "array.h"
#include "hashstring.h"

template<typename T, uint64_t width>
struct Dict
{
    Array<T>            m_data[width];
    Array<Hash>         m_keys[width];

    inline void Insert(Hash key, const T& item)
    {
        uint64_t slot = key % width;
        m_data[slot].grow() = item;
        m_keys[slot].grow() = key;
    }
    inline T* Get(Hash key)
    {
        uint64_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        return idx == 0xFFFF ? nullptr : &(m_data[slot][idx]);
    }
    inline const T* Get(Hash key) const
    {
        uint64_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        return idx == 0xFFFF ? nullptr : &(m_data[slot][idx]);
    }
    inline void Remove(Hash key)
    {
        uint64_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        if(idx != 0xFFFF)
        {
            m_data[slot].remove(idx);
            m_keys[slot].remove(idx);
        }
    }
};
