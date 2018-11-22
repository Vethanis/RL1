#pragma once

#include <mutex>
#include <condition_variable>

struct Semaphore
{
    std::mutex              m_mutex;
    std::condition_variable m_cvar;
    uint64_t                m_count = 0;

    void Signal() 
    {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        ++m_count;
        m_cvar.notify_one();
    }
    void Wait() 
    {
        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        while(!m_count)
        {
            m_cvar.wait(lock);
        }
        --m_count;
    }
    bool TryWait() 
    {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        if(m_count) 
        {
            --m_count;
            return true;
        }
        return false;
    }
};

struct LockGuard
{
    std::mutex& m_mutex;

    LockGuard(std::mutex& x) : m_mutex(x)
    {
        m_mutex.lock();
    }
    ~LockGuard()
    {
        m_mutex.unlock();
    }
};
