#pragma once

#include "macro.h"
#include "alloc.h"
#include "circular_stack.h"

struct LinearAllocator
{
    Allocation m_memory;
    usize m_head;
    CircularStack<Allocation, 64> m_stack;

    void Init(usize size);
    void Shutdown();
    void Reset();

    Allocation Allocate(usize bytes, usize align);
    Allocation Reallocate(Allocation prev, usize bytes, usize align);
    void Free(Allocation prev);
};
