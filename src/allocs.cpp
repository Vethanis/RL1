#include "allocs.h"

#include <malloc.h>
#include "memreq.h"
#include "linalloc.h"

namespace Malloc
{
    Allocation Allocate(size_t bytes, size_t align)
    {
        if(bytes == 0)
        {
            DebugInterrupt();
            return { nullptr, 0 };
        }

        const MemReq req = FixRequest(bytes, align);

        u8* newPtr = (u8*)_aligned_malloc(req.bytes, req.align);
        DebugAssert(newPtr);

        return { newPtr, req.bytes };
    }
    Allocation Reallocate(Allocation prev, size_t bytes, size_t align)
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

        const MemReq req = FixRequest(bytes, align);

        if(req.bytes <= prev.size())
        {
            return prev;
        }

        u8* newPtr = (u8*)_aligned_realloc(prev.begin(), req.bytes, req.align);
        DebugAssert(newPtr);

        return { newPtr, req.bytes };
    }
    void Free(Allocation x)
    {
        u8* ptr = x.begin();
        if(ptr)
        {
            _aligned_free(ptr);
        }
    }
};

namespace LinAlloc
{
    static LinearAllocator ms_linear;

    Allocation Alloc(size_t bytes, size_t align)
    {
        return ms_linear.Allocate(bytes, align);
    }
};