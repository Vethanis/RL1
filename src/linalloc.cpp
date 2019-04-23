#include "linalloc.h"

#include <malloc.h>
#include "memreq.h"

void LinearAllocator::Init(size_t size)
{
    EraseR(*this);

    if(size == 0)
    {
        DebugInterrupt();
        return;
    }
    
    const size_t newSize = AlignGrow(size, PageSize);

    uint8_t* ptr = (uint8_t*)_aligned_malloc(newSize, PageSize);
    DebugAssert(ptr);

    m_memory = { ptr, newSize };
}

void LinearAllocator::Shutdown()
{
    uint8_t* ptr = m_memory.begin();
    EraseR(*this);

    if(!ptr)
    {
        DebugInterrupt();
        return;
    }

    _aligned_free(ptr);
}

void LinearAllocator::Reset()
{
    Allocation alloc = m_memory;
    EraseR(*this);
    m_memory = alloc;
}

Allocation LinearAllocator::Allocate(size_t bytes, size_t align)
{
    if(bytes == 0)
    {
        DebugInterrupt();
        return { nullptr, 0 };
    }

    const MemReq req = FixRequest(bytes, align);
    const size_t start = AlignGrow(m_head, req.align);

    m_head = start + req.bytes;
    Allocation outAlloc = Subslice(m_memory, start, req.bytes);
    m_stack.push(outAlloc);

    return outAlloc;
}

Allocation LinearAllocator::Reallocate(Allocation prev, size_t bytes, size_t align)
{
    if(!prev.begin())
    {
        return this->Allocate(bytes, align);
    }
    if(bytes == 0)
    {
        this->Free(prev);
        return { nullptr, 0 };
    }

    const MemReq req = FixRequest(bytes, align);
    size_t start = AlignGrow(m_head, req.align);

    if(req.bytes <= prev.size())
    {
        return prev;
    }

    const Allocation lastAlloc = m_stack.peek();
    if(lastAlloc.begin() == prev.begin())
    {
        m_stack.pop();
        start = prev.begin() - m_memory.begin();
        start = AlignGrow(start, req.align);
    }

    m_head = start + req.bytes;
    Allocation outAlloc = Subslice(m_memory, start, req.bytes);
    m_stack.push(outAlloc);

    return outAlloc;
}

void LinearAllocator::Free(Allocation user)
{
    if(!user.begin())
    {
        return;
    }
    const Allocation prev = m_stack.peek();
    if(CompareV(user, prev) == 0)
    {
        m_stack.pop();
        const size_t start = prev.begin() - m_memory.begin();
        m_head = Min(m_head, start);
    }
}
