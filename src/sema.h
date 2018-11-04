#pragma once

struct Semaphore
{
    void Signal();
    void Wait();
    void Lock();
    void Unlock();
};

struct LockGuard
{
    Semaphore& sema;

    LockGuard(Semaphore& x) : sema(x)
    {
        sema.Lock();
    }
    ~LockGuard()
    {
        sema.Unlock();
    }
};
