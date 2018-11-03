#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

template<typename T, bool POD = true>
class Array
{
    T*      m_data;
    int32_t m_count;
    int32_t m_capacity;
public:
    Array(){ memset(this, 0, sizeof(*this)); }
    ~Array() { reset(); }
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
        if(!POD)
        {
            for(int32_t i = count() - 1; i >= new_size; --i)
            {
                (m_data + i)->~T();
            }
        }
        if(new_size > m_capacity)
        {
            reserve(new_size);
        }
        if(!POD)
        {
            for(int32_t i = count(); i < new_size; ++i)
            {
                new (m_data + i) T();
            }
        }
        m_count = new_size;
    }
    inline T& append()
    {
        ++m_count;
        T* item = &back();
        if(!POD)
        {
            new (item) T();
        }
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
        if(!POD)
        {
            T* item = &back();
            item->~T();
        }
        --m_count;
    }
    inline void clear() 
    {
        if(!POD)
        {
            for(int32_t i = 0; i < count(); ++i)
            {
                (m_data + i)->~T();
            }
        }
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
    int32_t find(const T& t)
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
    void assume(Array& other)
    {
        reset();
        memcpy(this, &other, sizeof(*this));
        memset(&other, 0, sizeof(*this));
    }
};
