#pragma once

#include "alloc.h"

namespace Malloc
{
    Allocation Allocate(size_t bytes, size_t align);
    Allocation Reallocate(Allocation prev, size_t bytes, size_t align);
    void Free(Allocation x);
};

namespace LinAlloc
{
    void Init();
    void Update();
    void Shutdown();

    Allocation Allocate(size_t bytes, size_t align);
    Allocation Reallocate(Allocation prev, size_t bytes, size_t align);
    void Free(Allocation x);
};

namespace DefaultAllocators
{
    static constexpr Allocator Malloc =
    {
        Malloc::Allocate,
        Malloc::Reallocate,
        Malloc::Free,
        256,
    };

    static constexpr Allocator Linear =
    {
        LinAlloc::Allocate,
        LinAlloc::Reallocate,
        LinAlloc::Free,
        64,
    };
};
