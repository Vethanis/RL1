#pragma once

#include "compalloc.h"
#include <new>

template<typename T, int32_t block_size = 64>
struct BlockAlloc : public ComponentAllocator
{
    struct Item
    {
        T    t;
        bool live;
    };
    Array<Item*> m_blocks;
    Array<Item*> m_free;

    BlockAlloc()
    {
        m_free.reserve(block_size);
    }
    ~BlockAlloc()
    {
        for(Item* block : m_blocks)
        {
            for(int32_t i = 0; i < block_size; ++i)
            {
                if(block[i].live)
                {
                    (block + i)->~Item();
                }
            }
            free(block);
        }
    }
    Component* Alloc() final
    {
        if(m_free.empty())
        {
            Item* items = (Item*)calloc(block_size, sizeof(Item));
            for(int32_t i = 0; i < block_size; ++i)
            {
                m_free.grow() = items + i;
            }
        }
        Item* c = m_free.back();
        m_free.pop();
        new (c) Item();
        c->live = true;
        return reinterpret_cast<T*>(c);
    }
    inline void Free(Component* c) final
    {
        Item* item = reinterpret_cast<Item*>(c);
        item->live = false;
        item->~Item();
        m_free.grow() = item;
    }
};
