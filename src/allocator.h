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
    void* Alloc(AllocBucket bucket, size_t bytes);
    void Free(AllocBucket bucket, void* p);
    void* Realloc(AllocBucket bucket, void* pOld, size_t oldBytes, size_t newBytes);
    void Update();

    template<typename T>
    T* Alloc(AllocBucket bucket)
    {
        return (T*)Alloc(bucket, sizeof(T));
    }
    template<typename T>
    inline T* New(AllocBucket bucket)
    {
        T* t = (T*)Alloc(bucket, sizeof(T));
        new (t) T();
        return t;
    }
    template<typename T>
    inline void Delete(AllocBucket bucket, T* t)
    {
        if(t)
        {
            t->~T();
            Free(bucket, (void*)t);
        }
    }
};
