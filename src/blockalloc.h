#pragma once

#include "compalloc.h"
#include <new>

template<typename T, int32_t block_size = 64>
struct BlockAlloc : public ComponentAllocator
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
    Component* Alloc() final
    {
        if(m_free.empty())
        {
            T* items = (T*)calloc(block_size, sizeof(T));
            m_blocks.grow() = items;
            for(int32_t i = 0; i < block_size; ++i)
            {
                m_free.grow() = items + i;
            }
        }
        T* t = m_free.back();
        m_free.pop();
        new (t) T();
        return t;
    }
    inline void Free(Component* c) final
    {
        T* t = static_cast<T*>(c);
        t->~T();
        m_free.grow() = t;
    }
};
