#include "allocs.h"

#include <stdlib.h>
#include "memreq.h"
#include "linalloc.h"

namespace Malloc
{
    Allocation Allocate(usize bytes, usize align)
    {
        if(bytes == 0)
        {
            DebugInterrupt();
            return { nullptr, 0 };
        }

        let req = FixRequest(bytes, align);

        u8* newPtr = (u8*)aligned_alloc(req.align, req.bytes);
        DebugAssert(newPtr);

        return { newPtr, req.bytes };
    }
    Allocation Reallocate(Allocation prev, usize bytes, usize align)
    {
        if(!prev.begin())
        {
            return Allocate(bytes, align);
        }
        if(bytes == 0)
        {
            DebugInterrupt();
            Free(prev);
            return { nullptr, 0 };
        }

        let req = FixRequest(bytes, align);

        if(req.bytes <= prev.size())
        {
            return prev;
        }

        u8* newPtr = (u8*)aligned_alloc(req.align, req.bytes);
        DebugAssert(newPtr);

        memcpy(newPtr, prev.begin(), prev.bytes());

        free(prev.begin());

        return { newPtr, req.bytes };
    }
    void Free(Allocation x)
    {
        u8* ptr = x.begin();
        if(ptr)
        {
            free(ptr);
        }
    }
};

namespace LinAlloc
{
    static constexpr usize ms_capacity = PageSize * 256;
    static LinearAllocator  ms_linear;

    void Init()
    {
        ms_linear.Init(ms_capacity);
    }
    void Update()
    {
        ms_linear.Reset();
    }
    void Shutdown()
    {
        ms_linear.Shutdown();
    }

    Allocation Allocate(usize bytes, usize align)
    {
        return ms_linear.Allocate(bytes, align);
    }
    Allocation Reallocate(Allocation prev, usize bytes, usize align)
    {
        return ms_linear.Reallocate(prev, bytes, align);
    }
    void Free(Allocation x)
    {
        ms_linear.Free(x);
    }
};
