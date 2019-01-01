#include "allocator.h"

#include <stdlib.h>
#include "macro.h"
#include "array.h"

struct BaseAllocator
{
    virtual ~BaseAllocator() {};
    virtual void* Alloc(size_t bytes) = 0;
    virtual void Free(void* p) = 0;
};

struct LinearAllocator : public BaseAllocator
{
    size_t   m_size;
    size_t   m_head;
    uint8_t* m_buffer;

    inline LinearAllocator(size_t size)
    {
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
        constexpr size_t align = 16;
        constexpr size_t alignMask = align - 1u;
        size_t misalignment = bytes & alignMask;
        size_t pad = (align - misalignment) & alignMask;
        bytes += pad;
        Assert(bytes + m_head <= m_size);
        void* p = m_buffer + m_head;
        m_head += bytes;
        return p;
    }
    inline void Free(void* p) final 
    {

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
};

DefaultAllocator    ms_default;
LinearAllocator     ms_temp(1ul << 30ul);
BaseAllocator*      ms_allocators[] = 
{
    &ms_default,
    &ms_temp,
};

namespace Allocator
{
    void* Alloc(AllocBucket bucket, size_t bytes)
    {
        return ms_allocators[bucket]->Alloc(bytes);
    }
    void Free(AllocBucket bucket, void* p)
    {
        return ms_allocators[bucket]->Free(p);
    }
    void Update()
    {
        ms_temp.Update();
    }
};
