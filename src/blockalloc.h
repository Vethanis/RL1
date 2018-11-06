#pragma once

#include <stdint.h>
#include <stdlib.h>

#include "array.h"

template<typename T, int32_t block_size = 64>
struct BlockAlloc
{
    Array<T*> m_blocks;
    Array<T*> m_free;

    BlockAlloc()
    {
        m_free.reserve(block_size);
    }
    ~BlockAlloc()
    {
        for(T* block : m_blocks)
        {
            free(block);
        }
    }
    T* Alloc()
    {
        if(m_free.count() == 0)
        {
            T* block = (T*)calloc(block_size, sizeof(T));
            for(int32_t i = block_size - 1; i >= 0; --i)
            {
                m_free.append() = block + i;
            }
        }
        T* item = m_free.back();
        m_free.pop();
        return item;
    }
    inline void Free(T* item)
    {
        m_free.grow() = item;
    }
};
