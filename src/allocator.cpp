#include "allocator.h"

#include <stdlib.h>
#include "macro.h"
#include "array.h"

inline size_t Align(size_t bytes)
{
    constexpr size_t align = 16;
    constexpr size_t alignMask = align - 1u;
    size_t misalignment = bytes & alignMask;
    size_t pad = (align - misalignment) & alignMask;
    bytes += pad;
    return bytes;
}

struct BaseAllocator
{
    virtual ~BaseAllocator() {};
    virtual void* Alloc(size_t bytes) = 0;
    virtual void Free(void* p) = 0;
    virtual void* Realloc(void* pOld, size_t oldBytes, size_t newBytes) = 0;
};

struct LinearAllocator : public BaseAllocator
{
    size_t   m_size;
    size_t   m_head;
    uint8_t* m_buffer;

    inline LinearAllocator(size_t size)
    {
        size = Align(size);
        m_size = size;
        m_head = 0;
        m_buffer = (uint8_t*)malloc(size);
        Assert(m_buffer);
    }
    ~LinearAllocator() final
    {
        free(m_buffer);
    }
    inline void* Alloc(size_t bytes) final
    {
        Assert(bytes + m_head <= m_size);
        void* p = m_buffer + m_head;
        m_head += bytes;
        return p;
    }
    inline void Free(void* p) final 
    {

    }
    inline void* Realloc(void* pOld, size_t oldBytes, size_t newBytes) final
    {
        uint8_t* oldAddr = (uint8_t*)pOld;
        if(oldAddr + oldBytes == m_buffer + m_head)
        {
            m_head -= oldBytes;
            m_head += newBytes;
            Assert(m_head <= m_size);
            return pOld;
        }

        void* pNew = Alloc(newBytes);
        memcpy(pNew, pOld, oldBytes);
        Free(pOld);
        return pNew;
    }
    inline void Update() 
    {
        m_head = 0;
    }
};

struct DefaultAllocator : public BaseAllocator
{
    inline void* Alloc(size_t bytes) final 
    {
        return malloc(bytes);
    }
    inline void Free(void* p) final 
    {
        free(p);
    }
    inline void* Realloc(void* pOld, size_t oldBytes, size_t newBytes)
    {
        return realloc(pOld, newBytes);
    }
};

static DefaultAllocator    ms_default;
static LinearAllocator     ms_temp(1ul << 30ul);
static BaseAllocator*      ms_allocators[] = 
{
    &ms_default,
    &ms_temp,
};

namespace Allocator
{
    void* Alloc(AllocBucket bucket, size_t bytes)
    {
        if(bytes)
        {
            bytes = Align(bytes);
            return ms_allocators[bucket]->Alloc(bytes);
        }
        return nullptr;
    }
    void Free(AllocBucket bucket, void* p)
    {
        if(p)
        {
            ms_allocators[bucket]->Free(p);
        }
    }
    void* Realloc(AllocBucket bucket, void* pOld, size_t oldBytes, size_t newBytes)
    {
        if(!newBytes)
        {
            Free(bucket, pOld);
            return nullptr;
        }
        if(!oldBytes || !pOld)
        {
            return Alloc(bucket, newBytes);
        }
        
        oldBytes = Align(oldBytes);
        newBytes = Align(newBytes);
        return ms_allocators[bucket]->Realloc(pOld, oldBytes, newBytes);
    }
    void Update()
    {
        ms_temp.Update();
    }
};
