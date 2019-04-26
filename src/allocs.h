#pragma once

#include "alloc.h"

namespace Malloc
{
    Allocation Allocate(usize bytes, usize align);
    Allocation Reallocate(Allocation prev, usize bytes, usize align);
    void Free(Allocation x);
};

namespace LinAlloc
{
    void Init();
    void Update();
    void Shutdown();

    Allocation Allocate(usize bytes, usize align);
    Allocation Reallocate(Allocation prev, usize bytes, usize align);
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
