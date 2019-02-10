#pragma once

#include <stdint.h>
#include <string.h>
#include <new>

#include "macro.h"
#include "allocator.h"

template<typename T, bool POD = true, AllocBucket t_bucket = AB_Default>
struct Array
{
    T*          m_data;
    int32_t     m_count;
    int32_t     m_capacity;

    inline Array()
    {
        MemZero(*this);
    }
    ~Array() { reset(); }
    Array(const T* x, int32_t ct)
    {
        MemZero(*this);
        resize(ct);
        if(!POD)
        {
            for(int32_t i = 0; i < count(); ++i)
            {
                m_data[i] = x[i];
            }
        }
        else
        {
            Copy(begin(), x, ct);
        }
    }
    Array(const Array& other)
    {
        MemZero(*this);
        resize(other.count());
        if(POD)
        {
            Copy(begin(), other.begin(), count());
        }
        else
        {
            for(int32_t i = 0; i < count(); ++i)
            {
                m_data[i] = other[i];
            }
        }
    }
    inline Array(Array&& other) noexcept
    {
        Assume(*this, other);
    }
    Array& operator=(const Array& other)
    {
        reset();
        resize(other.count());
        if(POD)
        {
            Copy(begin(), other.begin(), count());
        }
        else
        {
            for(int32_t i = 0; i < count(); ++i)
            {
                m_data[i] = other[i];
            }
        }
        return *this;
    }
    inline Array& operator=(Array&& other) noexcept 
    {
        reset();
        Assume(*this, other);
        return *this;
    }
    inline void assume(Array& other)
    {
        reset();
        Assume(*this, other);
    }
    inline int32_t capacity()   const { return m_capacity; }
    inline int32_t count()      const { return m_count; }
    inline bool full()          const { return count() == capacity(); }
    inline bool empty()         const { return count() == 0; }
    inline size_t bytes()       const { return sizeof(T) * (size_t)count(); }
    inline T* begin()                 { return m_data; }
    inline const T* begin()     const { return m_data; }
    inline T* end()                   { return m_data + count(); }
    inline const T* end()       const { return m_data + count(); }
    inline T& operator[](int32_t idx) { return m_data[idx]; }
    inline const T& operator[](int32_t idx) const { return m_data[idx]; }
    inline T& back()                  { return m_data[count() - 1u]; }
    inline const T& back()      const { return m_data[count() - 1u]; }
    inline void reserve(int32_t new_cap)
    {
        if(new_cap > capacity())
        {
            m_data = (T*)Allocator::Realloc(
                t_bucket, 
                m_data, 
                m_capacity * sizeof(T), 
                new_cap * sizeof(T));
            m_capacity = new_cap;
        }
    }
    inline void expand(int32_t step)
    {
        int32_t newCount = count() + step;
        if(newCount > capacity())
        {
            reserve(Max(capacity() * 2, newCount));
        }
    }
    void resize(const int32_t new_size)
    {
        if(new_size > m_capacity)
        {
            reserve(new_size);
        }
        if(!POD)
        {
            for(int32_t i = m_count - 1; i >= new_size; --i)
            {
                (m_data + i)->~T();
            }
            for(int32_t i = m_count; i < new_size; ++i)
            {
                new (m_data + i) T();
            }
        }
        m_count = new_size;
    }
    inline T& append()
    {
        Assert(count() < capacity());
        ++m_count;
        if(!POD)
        {
            new (&back()) T();
        }
        return back();
    }
    inline T& grow()
    {
        if(count() == capacity())
        {
            reserve(capacity() ? capacity() * 2 : 16);
        }
        ++m_count;
        if(!POD)
        {
            new (&back()) T();
        }
        return back();
    }
    inline void pop()
    {
        Assert(!empty());
        if(!POD)
        {
            (&back())->~T();
        }
        --m_count;
    }
    inline void clear() 
    {
        if(!POD)
        {
            for(int32_t i = 0; i < m_count; ++i)
            {
                (m_data + i)->~T();
            }
        }
        m_count = 0; 
    }
    inline void reset()
    {
        clear();
        Allocator::Free(t_bucket, m_data);
        MemZero(*this);
    }
    inline void remove(int32_t idx)
    {
        m_data[idx] = back();
        pop();
    }
    inline void shiftRemove(int32_t idx)
    {
        for(int32_t i = idx + 1; i < count(); ++i)
        {
            m_data[i - 1] = m_data[i];
        }
        pop();
    }
    int32_t find(const T& t) const
    {
        for(int32_t i = 0; i < count(); ++i)
        {
            if(m_data[i] == t)
                return i;
        }
        return -1;
    }
    inline int32_t findOrPush(const T& t)
    {
        int32_t idx = find(t);
        if(idx == -1)
        {
            idx = count();
            grow() = t;
        }
        return idx;
    }
    inline bool findRemove(const T& t)
    {
        int32_t idx = find(t);
        if(idx != -1)
        {
            remove(idx);
            return true;
        }
        return false;
    }
    inline int32_t largest() const
    {
        if(empty())
        {
            return -1;
        }
        int32_t i = 0;
        for(int32_t j = 1; j < count(); ++j)
        {
            if(m_data[j] > m_data[i])
            {
                i = j;
            }
        }
        return i;
    }
    inline int32_t smallest() const 
    {
        if(empty())
        {
            return -1;
        }
        int32_t i = 0;
        for(int32_t j = 1; j < count(); ++j)
        {
            if(m_data[j] < m_data[i])
            {
                i = j;
            }
        }
        return i;
    }
};

template<typename T, bool POD = true>
using TempArray = Array<T, POD, AB_Temp>;

template<typename A, typename B, AllocBucket t_bucket = AB_Default>
struct Array2
{
    uint8_t* m_data;
    int32_t  m_count;
    int32_t  m_capacity;

    inline Array2()
    {
        MemZero(*this);
    }
    inline Array2(const Array2& o)
    {
        MemZero(*this);
        resize(o.count());
        Copy(beginA(), o.beginA(), o.count());
        Copy(beginB(), o.beginB(), o.count());
    }
    inline Array2(Array2&& o) noexcept
    {
        Assume(*this, o);
    }
    inline Array2& operator=(const Array2& o)
    {
        reset();
        resize(o.count());
        Copy(beginA(), o.beginA(), o.count());
        Copy(beginB(), o.beginB(), o.count());
        return *this;
    }
    inline Array2& operator=(Array2&& o) noexcept
    {
        reset();
        Assume(*this, o);
        return *this;
    }
    ~Array2()
    {
        reset();
    }
    inline void reset()
    {
        Allocator::Free(t_bucket, m_data);
        MemZero(*this);
    }
    inline void clear()
    {
        m_count = 0;
    }
    inline int32_t count() const 
    {
        return m_count;
    }
    inline int32_t capacity() const 
    {
        return m_capacity;
    }
    inline A* beginA()
    {
        return (A*)m_data;
    }
    inline A* endA()
    {
        return beginA() + count();
    }
    inline B* beginB()
    {
        uint8_t* end = m_data + sizeof(A) * capacity();
        return (B*)end;
    }
    inline B* endB()
    {
        return beginB() + count();
    }
    inline const A* beginA() const
    {
        return (A*)m_data;
    }
    inline const A* endA() const
    {
        return beginA() + count();
    }
    inline const B* beginB() const
    {
        uint8_t* end = m_data + sizeof(A) * capacity();
        return (B*)end;
    }
    inline const B* endB() const
    {
        return beginB() + count();
    }
    inline A& backA()
    {
        return beginA()[count() - 1];
    }
    inline B& backB()
    {
        return beginB()[count() - 1];
    }
    inline const A& backA() const
    {
        return beginA()[count() - 1];
    }
    inline const B& backB() const
    {
        return beginB()[count() - 1];
    }
    inline A& getA(int32_t i)
    {
        return beginA()[i];
    }
    inline const A& getA(int32_t i) const
    {
        return beginA()[i];
    }
    inline B& getB(int32_t i)
    {
        return beginB()[i];
    }
    inline const B& getB(int32_t i) const
    {
        return beginB()[i];
    }
    void reserve(int32_t newCap)
    {
        if(newCap > capacity())
        {
            size_t bytes = sizeof(A) * newCap + sizeof(B) * newCap;
            uint8_t* data = (uint8_t*)Allocator::Alloc(t_bucket, bytes);

            Copy((A*)data, beginA(), count());
            Copy((B*)(data + sizeof(A) * newCap), beginB(), count());

            Allocator::Free(t_bucket, m_data);
            m_data = data;
            m_capacity = newCap;
        }
    }
    inline void resize(int32_t newCount)
    {
        if(newCount > capacity())
        {
            reserve(newCount);
        }
        m_count = newCount;
    }
    inline void expand(int32_t step)
    {
        int32_t newCount = count() + step;
        if(newCount > capacity())
        {
            reserve(Max(capacity() * 2, newCount));
        }
    }
    inline void grow()
    {
        if(count() == capacity())
        {
            reserve(capacity() ? capacity() * 2 : 16);
        }
        ++m_count;
    }
    inline void pop()
    {
        --m_count;
    }
    inline void remove(int32_t i)
    {
        getA(i) = backA();
        getB(i) = backB();
        --m_count;
    }
    void shiftRemove(int32_t i)
    {
        A* as = beginA();
        B* bs = beginB();
        for(int32_t j = i + 1; j < count(); ++j)
        {
            as[j - 1] = as[j];
        }
        for(int32_t j = i + 1; j < count(); ++j)
        {
            bs[j - 1] = bs[j];
        }
        pop();
    }
    int32_t findA(const A& x) const
    {
        const A* as = beginA();
        for(int32_t i = 0; i < count(); ++i)
        {
            if(as[i] == x)
            {
                return i;
            }
        }
        return -1;
    }
    int32_t findB(const B& x) const
    {
        const B* bs = beginB();
        for(int32_t i = 0; i < count(); ++i)
        {
            if(bs[i] == x)
            {
                return i;
            }
        }
        return -1;
    }
};

template<typename T, int32_t m_capacity>
struct FixedArray
{
    T          m_data[m_capacity];
    int32_t    m_count;

    inline int32_t capacity()   const { return m_capacity; }
    inline int32_t count()      const { return m_count; }
    inline bool full()          const { return count() == capacity(); }
    inline bool empty()         const { return count() == 0; }
    inline size_t bytes()       const { return sizeof(T) * (size_t)count(); }
    inline T* begin()                 { return m_data; }
    inline const T* begin()     const { return m_data; }
    inline T* end()                   { return m_data + m_count; }
    inline const T* end()       const { return m_data + m_count; }
    inline T& operator[](int32_t idx) { return m_data[idx]; }
    inline const T& operator[](int32_t idx) const { return m_data[idx]; }
    inline T& back()                  { return m_data[count() - 1u]; }
    inline const T& back()      const { return m_data[count() - 1u]; }
    inline void resize(const int32_t new_size)
    {
        Assert(new_size <= capacity());
        m_count = new_size;
    }
    inline T& append()
    {
        Assert(count() < capacity());
        ++m_count;
        return back();
    }
    inline T& grow()
    {
        return append();
    }
    inline void pop()
    {
        Assert(!empty());
        --m_count;
    }
    inline void clear() 
    {
        m_count = 0; 
    }
    inline void reset()
    {
        clear();
    }
    inline void remove(int32_t idx)
    {
        m_data[idx] = back();
        pop();
    }
    inline void shiftRemove(int32_t idx)
    {
        for(int32_t i = idx + 1; i < count(); ++i)
        {
            m_data[i - 1] = m_data[i];
        }
        pop();
    }
    int32_t find(const T& t) const
    {
        for(int32_t i = 0; i < count(); ++i)
        {
            if(m_data[i] == t)
                return i;
        }
        return -1;
    }
    inline int32_t findOrPush(const T& t)
    {
        int32_t idx = find(t);
        if(idx == -1)
        {
            idx = count();
            grow() = t;
        }
        return idx;
    }
    inline bool findRemove(const T& t)
    {
        int32_t idx = find(t);
        if(idx != -1)
        {
            remove(idx);
            return true;
        }
        return false;
    }
    inline int32_t largest() const
    {
        if(empty())
        {
            return -1;
        }
        int32_t i = 0;
        for(int32_t j = 1; j < count(); ++j)
        {
            if(m_data[j] > m_data[i])
            {
                i = j;
            }
        }
        return i;
    }
    inline int32_t smallest() const 
    {
        if(empty())
        {
            return -1;
        }
        int32_t i = 0;
        for(int32_t j = 1; j < count(); ++j)
        {
            if(m_data[j] < m_data[i])
            {
                i = j;
            }
        }
        return i;
    }
};
