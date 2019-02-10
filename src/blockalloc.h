#pragma once

#include "array.h"
#include <stdlib.h>
#include <new>

template<typename T, int32_t start_size = 8>
struct TBlockAlloc
{
    Array<T*> m_blocks;
    Array<T*> m_free;
    int32_t   m_blockSize;

    TBlockAlloc()
    {
        m_blockSize = start_size;
    }
    ~TBlockAlloc()
    {
        Reset();
    }
    void Reset()
    {
        for(T* block : m_blocks)
        {
            free(block);
        }
        m_free.reset();
        m_blocks.reset();
        m_blockSize = start_size;
    }
    T* Alloc()
    {
        if(m_free.empty())
        {
            T* items = (T*)calloc(m_blockSize, sizeof(T));
            m_blocks.grow() = items;
            m_free.expand(m_blockSize);
            for(int32_t i = m_blockSize - 1; i >= 0; --i)
            {
                m_free.append() = items + i;
            }
            m_blockSize *= 2;
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

template<typename T, int32_t start_size = 8>
struct TBlockAllocCD
{
    TBlockAlloc<T, start_size> m_block;

    inline T* Alloc()
    {
        T* t = m_block.Alloc();
        new (t) T();
        return t;
    }
    inline void Free(T* c)
    {
        if(c)
        {
            c->~T();
            m_block.Free(c);
        }
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
    void Reset(int32_t blockSize = 8)
    {
        for(uint8_t* block : m_blocks)
        {
            free(block);
        }
        m_free.reset();
        m_blocks.reset();
        m_blockSize = blockSize;
    }
    void* Alloc()
    {
        if(m_free.empty())
        {
            uint8_t* items = (uint8_t*)calloc(m_blockSize, m_itemSize);
            m_blocks.grow() = items;
            m_free.expand(m_blockSize);
            for(int32_t i = m_blockSize - 1; i >= 0; --i)
            {
                m_free.append() = items + i * m_itemSize;
            }
            m_blockSize *= 2;
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
