#pragma once

#include "macro.h"
#include "memory.h"
#include "allocs.h"

template<typename T, const Allocator& t_alloc = DefaultAllocators::Malloc>
struct Array
{
    Slice<T> m_mem;
    size_t   m_size;
    
    inline       T* begin()          { return m_mem.begin(); }
    inline const T* begin()    const { return m_mem.begin(); }
    inline size_t   capacity() const { return m_mem.size();  }
    inline size_t   size()     const { return m_size;        }
    
    inline size_t capacity_bytes() const { return capacity() * sizeof(T); }
    inline size_t size_bytes()     const { return m_size * sizeof(T); }

    inline bool full()  const { return m_size == capacity(); }
    inline bool empty() const { return m_size == (size_t)0; }

    inline       T* end()       { return begin() + m_size; }
    inline const T* end() const { return begin() + m_size; }

    inline T& front() 
    {
        DebugAssert(!empty());
        return m_mem[0];
    }
    inline const T& front() const 
    {
        DebugAssert(!empty());
        return m_mem[0];
    }

    inline T& back() 
    {
        DebugAssert(!empty());
        return m_mem[m_size - 1];
    }
    inline const T& back() const 
    {
        DebugAssert(!empty());
        return m_mem[m_size - 1];
    }

    inline T& operator[](size_t i) 
    {
        DebugAssert(i < m_size); 
        return m_mem[i];
    }
    inline const T& operator[](size_t i) const 
    {
        DebugAssert(i < m_size);
        return m_mem[i];
    }

    inline void clear()
    {
        m_size = 0;
    }
    inline T& append()
    {
        DebugAssert(!full());
        return m_mem[m_size++];
    }
    inline void pop()
    {
        DebugAssert(!empty());
        --m_size;
    }
    inline T pop_value()
    {
        DebugAssert(!empty());
        return m_mem[m_size--];
    }
    inline void remove(size_t i)
    {
        DebugAssert(i < m_size);
        Copy(m_mem[i], back());
        pop();
    }
    inline void remove(const T* ptr)
    {
        DebugAssert(ptr < end() && ptr >= begin());
        Copy(*ptr, back());
        pop();
    }

    inline void reset()
    {
        t_alloc.Free<T>(m_mem);
        m_size = 0;
    }
    inline void reserve(size_t newCap)
    {
        t_alloc.Realloc<T>(m_mem, newCap);
    }
    inline void expand(size_t step)
    {
        reserve(m_size + step);
    }
    inline T& grow()
    {
        if(full())
        {
            reserve(m_size + 1);
        }
        return m_mem[m_size++];
    }
    inline void resize(size_t count)
    {
        reserve(count);
        m_size = count;
    }
    inline void fill(Slice<const T> src) 
    {
        resize(src.size());
        Copy(begin(), src.begin(), src.size());
    }

    inline Slice<T> slice()
    {
        return { begin(), size() };
    }
    inline Slice<const T> cslice() const
    {
        return { begin(), size() };
    }
};

template<typename T>
using LinearArray = Array<T, DefaultAllocators::Linear>;

template<typename T, const Allocator& t_alloc>
inline Array<T, t_alloc> ToArray(Slice<const T> x) 
{
    Array<T, t_alloc> y;
    Erase(y);
    y.resize(x.size());
    y.fill(x);
    return y;
}
template<typename T, const Allocator& t_alloc>
inline Array<T, t_alloc> ToArray(Slice<T> x) 
{
    Array<T, t_alloc> y;
    Erase(y);
    y.resize(x.size());
    y.fill(x.to_const());
    return y;
}

template<typename T, const Allocator& t_alloc>
inline Slice<T> ToSlice(Array<T, t_alloc>& x)
{
    return { x.begin(), x.size() };
}
template<typename T, const Allocator& t_alloc>
inline Slice<const T> ToCSlice(const Array<T, t_alloc>& x)
{
    return { x.begin(), x.size() };
}
