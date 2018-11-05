#pragma once

#include <stdint.h>

#include "array.h"
#include "thread.h"
#include "sema.h"
#include "sokol_time.h"

struct Task
{
    void (*fn)(Task*);
    uint64_t mem[3];
};

struct TaskManager
{
    Thread          threads[4];
    Semaphore       sema;
    Array<Task>     tasks;
    uint64_t        duration;

    // will use up to ms milliseconds to do parallel updates; can early out
    void SetDuration(uint64_t ms)
    {
        duration = ms * 1000000ull;
    }
    // begins multithreaded phase of engine frame; blocking
    void Start()
    {
        for(Thread& t : threads)
        {
            t = Thread(SRun, this);
        }
        for(Thread& t : threads)
        {
            t.Join();
        }
    }
    // add task to task stack (not a queue!)
    void Add(const Task& task)
    {
        tasks.grow() = task;
    }
    // internal
    static void SRun(void* data)
    {
        TaskManager* m = (TaskManager*)data;
        m->Run();
    }
    // internal
    void Run()
    {
        uint64_t started = stm_now();
        while(true)
        {
            uint64_t now = stm_now();
            if(now - started >= duration)
            {
                break;
            }

            Task t;
            {
                LockGuard guard(sema);
                if(tasks.empty())
                {
                    break;
                }
                t = tasks.back();
                tasks.pop();
            }
            t.fn(&t);
        }
    }
};
