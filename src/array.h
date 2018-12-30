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

    Array()
    { 
        memset(this, 0, sizeof(*this)); 
    }
    ~Array() { reset(); }
    Array(const T* x, int32_t ct)
    {
        memset(this, 0, sizeof(*this)); 
        resize(ct);
        memcpy(m_data, x, sizeof(T) * ct);
    }
    Array(const Array& other)
    {
        memset(this, 0, sizeof(*this));
        resize(other.count());
        if(POD)
        {
            memcpy(begin(), other.begin(), bytes());
        }
        else
        {
            for(int32_t i = 0; i < count(); ++i)
            {
                m_data[i] = other[i];
            }
        }
    }
    Array(Array&& other) noexcept
    {
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
    Array& operator=(const Array& other)
    {
        reset();
        resize(other.count());
        if(POD)
        {
            memcpy(begin(), other.begin(), bytes());
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
    Array& operator=(Array&& other) noexcept 
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
        return *this;
    }
    void assume(Array& other)
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
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
    inline void reserve(int32_t new_cap)
    {
        if(new_cap > capacity())
        {
            T* data = (T*)Allocator::Alloc(t_bucket, sizeof(T) * new_cap);
            memcpy(data, m_data, sizeof(T) * count());
            Allocator::Free(t_bucket, m_data);
            m_data = data;
            m_capacity = new_cap;
        }
    }
    inline void expand(int32_t step)
    {
        const int32_t newCount = count() + step;
        if(newCount > capacity())
        {
            int32_t newCap = capacity() * 2;
            if(newCount > newCap)
            {
                newCap += step;
            }
            reserve(newCap);
        }
        // dont update count, let user append
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
        if(count() == capacity())
        {
            Assert(false);
        }
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
            reserve(count() ? count() * 2 : 16);
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
        memset(this, 0, sizeof(*this));
    }
    inline void remove(int32_t idx)
    {
        m_data[idx] = back();
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
};

template<typename T, bool POD = true>
using TempArray = Array<T, POD, AB_Temp>;

template<typename T, int32_t m_capacity>
struct FixedArray
{
    T          m_data[m_capacity];
    int32_t    m_count;

    FixedArray(){ memset(this, 0, sizeof(*this)); }
    ~FixedArray() { reset(); }
    FixedArray(const FixedArray& other)
    {
        memset(this, 0, sizeof(*this));
        resize(other.count());
        memcpy(begin(), other.begin(), bytes());
    }
    FixedArray(FixedArray&& other) noexcept
    {
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
    FixedArray(const T* x, int32_t count)
    {
        Assert(count <= m_capacity);
        for(int32_t i = 0; i < count; ++i)
        {
            m_data[i] = x[i];
        }
        m_count = count;
    }
    FixedArray& operator=(const FixedArray& other)
    {
        reset();
        resize(other.count());
        memcpy(begin(), other.begin(), bytes());
        return *this;
    }
    FixedArray& operator=(FixedArray&& other) noexcept 
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
        return *this;
    }
    void assume(FixedArray& other)
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
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
    void resize(const int32_t new_size)
    {
        if(new_size > capacity())
        {
            Assert(false);
        }
        m_count = new_size;
    }
    inline T& append()
    {
        if(count() == capacity())
        {
            Assert(false);
        }
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
        memset(this, 0, sizeof(*this));
    }
    inline void remove(int32_t idx)
    {
        m_data[idx] = back();
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
};
