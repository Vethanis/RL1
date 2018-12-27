#include "allocator.h"

#include <stdlib.h>
#include "macro.h"
#include "array.h"

struct BaseAllocator
{
    virtual ~BaseAllocator() {};
    virtual void* Alloc(size_t bytes) = 0;
    virtual void Free(void* p) = 0;
    virtual void Update() {};
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
        Assert(bytes + m_head <= m_size);
        void* p = m_buffer + m_head;
        m_head += bytes;
        return p;
    }
    inline void Free(void* p) final 
    {

    }
    inline void Update() final 
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
LinearAllocator     ms_stack(1ul << 30ul);
BaseAllocator*      ms_allocators[] = 
{
    &ms_default,
    &ms_temp,
    &ms_stack,
};
AllocBucket defaultBucket[] = { AB_Default };
FixedArray<AllocBucket, 256> ms_bucket(defaultBucket, NELEM(defaultBucket));

namespace Allocator
{
    void PushBucket(AllocBucket bucket)
    {
        ms_bucket.append() = bucket;
    }
    void PopBucket()
    {
        ms_bucket.pop();
    }
    AllocBucket GetCurrent()
    {
        return ms_bucket.back();
    }
    void* Alloc(size_t bytes)
    {
        return ms_allocators[GetCurrent()]->Alloc(bytes);
    }
    void Free(void* p)
    {
        return ms_allocators[GetCurrent()]->Free(p);
    }
    void Update()
    {
        for(BaseAllocator* p : ms_allocators)
        {
            p->Update();
        }
    }
};

BucketScopeStack::BucketScopeStack()
{
    m_head = ms_temp.m_head;
    Allocator::PushBucket(AB_Stack);
}

BucketScopeStack::~BucketScopeStack()
{
    ms_temp.m_head = m_head;
    Allocator::PopBucket();
}
