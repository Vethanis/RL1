#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

template<typename T>
struct Array
{
    T*      m_data;
    int32_t m_count;
    int32_t m_capacity;

    Array(){ memset(this, 0, sizeof(*this)); }
    ~Array() { reset(); }
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
    inline T& back()                  { return m_data[count() - 1]; }
    inline const T& back()      const { return m_data[count() - 1]; }
    inline void reserve(int32_t new_cap)
    {
        if(new_cap > capacity())
        {
            m_data = (T*)realloc(m_data, sizeof(T) * new_cap);
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
    inline T& append()
    {
        ++m_count;
        T* item = &back();
        return *item;
    }
    inline T& grow()
    {
        if(count() == capacity())
        {
            reserve(count() ? count() * 2 : 16);
        }
        return append();
    }
    inline void pop()
    {
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
    inline void uniquePush(const T& t)
    {
        if(find(t) == -1)
        {
            grow() = t;
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
    inline T& backT() { return m_ts[count() - 1]; }
    inline const T& backT() const { return m_ts[count() - 1]; }
    inline U& backU() { return m_us[count() - 1]; }
    inline const U& backU() const { return m_us[count() - 1]; }
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
        ++m_count;
    }
    inline void grow()
    {
        if(count() == capacity())
        {
            reserve(count() ? count() * 2 : 16);
        }
        append();
    }
    inline void pop()
    {
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
    inline void uniquePush(const T& t)
    {
        if(find(t) == -1)
        {
            grow() = t;
        }
    }
    inline void uniquePush(const U& u)
    {
        if(find(u) == -1)
        {
            grow();
            m_us[count() - 1] = u;
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
