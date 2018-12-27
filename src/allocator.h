#pragma once

#include <stdint.h>
#include <new>

enum AllocBucket
{
    AB_Default = 0,
    AB_Temp,
};

namespace Allocator
{
    void PushBucket(AllocBucket bucket);
    void PopBucket();
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
        if(t)
        {
            t->~T();
            Free((void*)t);
        }
    }
};

// sets mem bucket for a scope
struct BucketScope
{
    BucketScope(AllocBucket bucket)
    {
        Allocator::PushBucket(bucket);
    }
    ~BucketScope()
    {
        Allocator::PopBucket();
    }
};

// sets temp mem bucket and frees all mem allocated in scope
struct TempMemScope
{
    size_t m_head;
    TempMemScope();
    ~TempMemScope();
};
