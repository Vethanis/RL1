#pragma once

#include "array.h"

template<typename T, uint32_t width>
struct Dict
{
    Array<T>        m_data[width];
    Array<uint32_t> m_keys[width];

    inline void Insert(uint32_t key, const T& item)
    {
        uint32_t slot = key % width;
        m_data[slot].grow() = item;
        m_keys[slot].grow() = key;
    }
    inline T* Get(uint32_t key)
    {
        uint32_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        return idx == 0xFFFF ? nullptr : &(m_data[slot][idx]);
    }
    inline const T* Get(uint32_t key) const
    {
        uint32_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        return idx == 0xFFFF ? nullptr : &(m_data[slot][idx]);
    }
    inline void Remove(uint32_t key)
    {
        uint32_t slot = key % width;
        uint16_t idx = m_keys[slot].find(key);
        if(idx != 0xFFFF)
        {
            m_data[slot].remove(idx);
            m_keys[slot].remove(idx);
        }
    }
};
