#pragma once

#include <stdint.h>
#include <new>

enum AllocBucket
{
    AB_Default = 0,
    AB_Temp,
    AB_Stack,
};

namespace Allocator
{
    void PushBucket(AllocBucket bucket);
    void PopBucket();
    AllocBucket GetCurrent();
    void* Alloc(size_t bytes);
    void Free(void* p);
    void Update();

    template<typename T>
    T* Alloc()
    {
        return (T*)Alloc(sizeof(T));
    }

    template<typename T>
    inline T* New()
    {
        T* t = (T*)Alloc(sizeof(T));
        new (t) T();
        return t;
    }

    template<typename T>
    inline void Delete(T* t)
    {
        if(t && GetCurrent() < AB_Temp)
        {
            // dont call destructors on temp mem, its a waste of cache
            t->~T();
            Free((void*)t);
        }
    }
};

// sets mem bucket for a scope
struct BucketScope
{
    inline BucketScope(AllocBucket bucket)
    {
        Allocator::PushBucket(bucket);
    }
    inline ~BucketScope()
    {
        Allocator::PopBucket();
    }
};

struct BucketScopeDefault
{
    inline BucketScopeDefault()
    {
        Allocator::PushBucket(AB_Default);
    }
    inline ~BucketScopeDefault()
    {
        Allocator::PopBucket();
    }
};

struct BucketScopeTemp
{
    inline BucketScopeTemp()
    {
        Allocator::PushBucket(AB_Temp);
    }
    inline ~BucketScopeTemp()
    {
        Allocator::PopBucket();
    }
};

// stack scope memory
struct BucketScopeStack
{
    size_t m_head;
    BucketScopeStack();
    ~BucketScopeStack();
};
