#pragma once

#include "alloc.h"
#include "circular_stack.h"

struct LinearAllocator
{
    Allocation m_memory;
    size_t m_head;
    CircularStack<Allocation, 64> m_stack;

    void Init(size_t size);
    void Shutdown();
    void Reset();

    Allocation Allocate(size_t bytes, size_t align);
    Allocation Reallocate(Allocation prev, size_t bytes, size_t align);
    Allocation Free(Allocation prev);
};
