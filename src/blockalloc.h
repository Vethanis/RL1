#pragma once

#include "array.h"
#include <stdlib.h>

template<typename T, int32_t block_size = 64>
struct TBlockAlloc
{
    Array<T*> m_blocks;
    Array<T*> m_free;

    TBlockAlloc()
    {
        m_free.reserve(block_size);
    }
    ~TBlockAlloc()
    {
        for(T* block : m_blocks)
        {
            free(block);
        }
    }
    T* Alloc()
    {
        if(m_free.empty())
        {
            T* items = (T*)calloc(block_size, sizeof(T));
            m_blocks.grow() = items;
            for(int32_t i = block_size - 1; i >= 0; --i)
            {
                m_free.grow() = items + i;
            }
        }
        T* t = m_free.back();
        m_free.pop();
        memset(t, 0, sizeof(T));
        return t;
    }
    inline void Free(T* c)
    {
        m_free.grow() = c;
    }
};

struct BlockAlloc
{
    Array<void*>    m_free;
    Array<uint8_t*> m_blocks;
    int32_t         m_itemSize;
    int32_t         m_blockSize;

    template<typename T>
    void Init(int32_t blockSize = 64)
    {
        m_itemSize = sizeof(T);
        m_blockSize = blockSize;
        m_free.reserve(m_blockSize);
    }
    ~BlockAlloc()
    {
        for(uint8_t* block : m_blocks)
        {
            free(block);
        }
    }
    void* Alloc()
    {
        if(m_free.empty())
        {
            uint8_t* items = (uint8_t*)calloc(m_blockSize, m_itemSize);
            m_blocks.grow() = items;
            for(int32_t i = m_blockSize - 1; i >= 0; --i)
            {
                m_free.grow() = items + i * m_itemSize;
            }
        }
        void* t = m_free.back();
        m_free.pop();
        memset(t, 0, m_itemSize);
        return t;
    }
    inline void Free(void* c)
    {
        m_free.grow() = c;
    }
};
