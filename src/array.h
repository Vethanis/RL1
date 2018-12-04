#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "macro.h"

template<typename T>
struct Array
{
    T*          m_data;
    int32_t     m_count;
    int32_t     m_capacity;

    Array(){ memset(this, 0, sizeof(*this)); }
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
        memcpy(begin(), other.begin(), bytes());
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
        memcpy(begin(), other.begin(), bytes());
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
            m_data = (T*)realloc(m_data, sizeof(T) * new_cap);
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
            m_data = (T*)realloc(m_data, sizeof(T) * newCap);
            m_capacity = newCap;
        }
        // dont update count, let user append
    }
    void resize(const int32_t new_size)
    {
        if(new_size > m_capacity)
        {
            reserve(new_size);
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
        if(count() == capacity())
        {
            reserve(count() ? count() * 2 : 16);
        }
        ++m_count;
        return back();
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
        free(m_data);
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

template<typename T, typename U>
struct Array2
{
    T* m_ts;
    U* m_us;
    int32_t m_count;
    int32_t m_capacity;

    Array2() { memset(this, 0, sizeof(*this)); }
    ~Array2() { reset(); }
    Array2(const Array2& other)
    {
        memset(this, 0, sizeof(*this));
        resize(other.count());
        memcpy(beginT(), other.beginT(), sizeof(T) * count());
        memcpy(beginU(), other.beginU(), sizeof(U) * count());
    }
    Array2(Array2&& other) noexcept
    {
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
    Array2& operator=(const Array2& other)
    {
        reset();
        resize(other.count());
        memcpy(beginT(), other.beginT(), sizeof(T) * count());
        memcpy(beginU(), other.beginU(), sizeof(U) * count());
        return *this;
    }
    Array2& operator=(Array2&& other) noexcept 
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
        return *this;
    }
    void assume(Array2& other)
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
    inline int32_t capacity()   const { return m_capacity; }
    inline int32_t count()      const { return m_count; }
    inline T* beginT() { return m_ts; }
    inline T* endT() { return m_ts + count(); }
    inline U* beginU() { return m_us; }
    inline U* endU() { return m_us + count(); }
    inline const T* beginT() const { return m_ts; }
    inline const T* endT() const { return m_ts + count(); }
    inline const U* beginU() const { return m_us; }
    inline const U* endU() const { return m_us + count(); }
    inline T& backT() { return m_ts[count() - 1u]; }
    inline const T& backT() const { return m_ts[count() - 1u]; }
    inline U& backU() { return m_us[count() - 1u]; }
    inline const U& backU() const { return m_us[count() - 1u]; }
    inline T& GetT(int32_t i)
    {
        return m_ts[i];
    }
    inline const T& GetT(int32_t i) const
    {
        return m_ts[i];
    }
    inline U& GetU(int32_t i)
    {
        return m_us[i];
    }
    inline const U& GetU(int32_t i) const
    {
        return m_us[i];
    }
    inline void reserve(int32_t new_cap)
    {
        if(new_cap > capacity())
        {
            m_ts = (T*)realloc(m_ts, sizeof(T) * new_cap);
            m_us = (U*)realloc(m_us, sizeof(U) * new_cap);
            m_capacity = new_cap;
        }
    }
    void resize(const int32_t new_size)
    {
        if(new_size > m_capacity)
        {
            reserve(new_size);
        }
        m_count = new_size;
    }
    inline void append()
    {
        if(count() == capacity())
        {
            Assert(false);
        }
        ++m_count;
    }
    inline void grow()
    {
        if(count() == capacity())
        {
            reserve(count() ? count() * 2 : 16);
        }
        ++m_count;
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
        free(m_ts);
        free(m_us);
        memset(this, 0, sizeof(*this));
    }
    inline void remove(int32_t idx)
    {
        m_ts[idx] = backT();
        m_us[idx] = backU();
        pop();
    }
    int32_t find(const T& t) const
    {
        for(int32_t i = 0; i < count(); ++i)
        {
            if(m_ts[i] == t)
                return i;
        }
        return -1;
    }
    int32_t find(const U& u) const
    {
        for(int32_t i = 0; i < count(); ++i)
        {
            if(m_us[i] == u)
                return i;
        }
        return -1;
    }
    inline void findOrPush(const T& t)
    {
        if(find(t) == -1)
        {
            grow() = t;
        }
    }
    inline void findOrPush(const U& u)
    {
        if(find(u) == -1)
        {
            grow();
            m_us[count() - 1u] = u;
        }
    }
    inline void findRemove(const T& t)
    {
        int32_t idx = find(t);
        if(idx != -1)
        {
            remove(idx);
        }
    }
    inline void findRemove(const U& u)
    {
        int32_t idx = find(u);
        if(idx != -1)
        {
            remove(idx);
        }
    }
};

template<typename T, int32_t m_capacity>
struct FixedArray
{
    T           m_data[m_capacity];
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
