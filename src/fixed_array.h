#pragma once

#include "lang.h"
#include "slice.h"

template<typename T, usize t_capacity>
struct FixedArray
{
    T m_mem[t_capacity];
    usize m_size;

    usize   capacity() const { return t_capacity; }
    usize   size()     const { return m_size;     }
          T* begin()          { return m_mem;     }
    const T* begin()    const { return m_mem;     }

    bool full()  const { return m_size == t_capacity; }
    bool empty() const { return m_size == 0; }

    usize capacity_bytes() const { return sizeof(T) * t_capacity; }
    usize size_bytes()     const { return sizeof(T) * m_size; }

          T* end()       { return begin() + m_size; }
    const T* end() const { return begin() + m_size; }

    T& operator[](usize idx)
    {
        DebugAssert(idx < m_size);
        return m_mem[idx];
    }
    const T& operator[](usize idx) const
    {
        DebugAssert(idx < m_size);
        return m_mem[idx];
    }

    T& front()
    {
        DebugAssert(!empty());
        return m_mem[0];
    }
    const T& front() const
    {
        DebugAssert(!empty());
        return m_mem[0];
    }

    T& back()
    {
        DebugAssert(!empty());
        return m_mem[m_size - 1];
    }
    const T& back() const
    {
        DebugAssert(!empty());
        return m_mem[m_size - 1];
    }

    void resize(usize newSize)
    {
        DebugAssert(newSize <= t_capacity);
        m_size = newSize;
    }
    T& append()
    {
        DebugAssert(!full());
        return m_mem[m_size++];
    }
    void pop()
    {
        DebugAssert(!empty());
        --m_size;
    }
    T pop_value()
    {
        DebugAssert(!empty());
        return m_mem[m_size--];
    }
    void clear()
    {
        m_size = 0;
    }
    void remove(usize i)
    {
        DebugAssert(i < m_size);
        Copy(m_mem[i], back());
        pop();
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
