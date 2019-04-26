#include "linalloc.h"

#include <stdlib.h>
#include "memreq.h"

void LinearAllocator::Init(usize size)
{
    EraseR(*this);

    if(size == 0)
    {
        DebugInterrupt();
        return;
    }

    let newSize = AlignGrow(size, PageSize);

    u8* ptr = (u8*)aligned_alloc(PageSize, newSize);
    DebugAssert(ptr);

    m_memory = { ptr, newSize };
}

void LinearAllocator::Shutdown()
{
    u8* ptr = m_memory.begin();
    EraseR(*this);

    if(!ptr)
    {
        DebugInterrupt();
        return;
    }

    free(ptr);
}

void LinearAllocator::Reset()
{
    Allocation alloc = m_memory;
    EraseR(*this);
    m_memory = alloc;
}

Allocation LinearAllocator::Allocate(usize bytes, usize align)
{
    if(bytes == 0)
    {
        DebugInterrupt();
        return { nullptr, 0 };
    }

    let req = FixRequest(bytes, align);
    let start = AlignGrow(m_head, req.align);

    m_head = start + req.bytes;
    Allocation outAlloc = Subslice(m_memory, start, req.bytes);
    m_stack.push(outAlloc);

    return outAlloc;
}

Allocation LinearAllocator::Reallocate(Allocation prev, usize bytes, usize align)
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

    let req = FixRequest(bytes, align);
    letmut start = AlignGrow(m_head, req.align);

    if(req.bytes <= prev.size())
    {
        return prev;
    }

    let lastAlloc = m_stack.peek();
    if(lastAlloc.begin() == prev.begin())
    {
        m_stack.pop();
        start = prev.begin() - m_memory.begin();
        start = AlignGrow(start, req.align);
    }

    m_head = start + req.bytes;
    letmut outAlloc = Subslice(m_memory, start, req.bytes);
    m_stack.push(outAlloc);

    return outAlloc;
}

void LinearAllocator::Free(Allocation user)
{
    if(!user.begin())
    {
        return;
    }
    let prev = m_stack.peek();
    if(CompareV(user, prev) == 0)
    {
        m_stack.pop();
        usize start = prev.begin() - m_memory.begin();
        m_head = Min(m_head, start);
    }
}
