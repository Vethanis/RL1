#pragma once

#include <mutex>
#include <condition_variable>

struct LockGuard
{
    std::mutex& m_mutex;

    inline LockGuard(std::mutex& x) : m_mutex(x)
    {
        m_mutex.lock();
    }
    inline ~LockGuard()
    {
        m_mutex.unlock();
    }
};


struct Semaphore
{
    std::mutex              m_mutex;
    std::condition_variable m_cvar;
    uint64_t                m_count = 0;

    inline void Signal(int32_t count = 1) 
    {
        LockGuard lock(m_mutex);
        for(int32_t i = 0; i < count; ++i)
        {
            ++m_count;
            m_cvar.notify_one();
        }
    }
    inline void Wait(int32_t count = 1) 
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        for(int32_t i = 0; i < count; ++i)
        {
            while(!m_count)
            {
                m_cvar.wait(lock);
            }
            --m_count;
        }
    }
    inline bool TryWait() 
    {
        LockGuard lock(m_mutex);
        if(m_count) 
        {
            --m_count;
            return true;
        }
        return false;
    }
};
