#pragma once

#include <stdint.h>

#include "array.h"
#include "thread.h"
#include "sema.h"
#include "comptype.h"
#include "sokol_time.h"

struct Task
{
    void (*fn)(Task*);
    uint64_t mem[3];
};

struct TaskManager
{
    // will use up to ms milliseconds to do parallel updates; can early out
    // begins multithreaded phase of engine frame; blocking
    static void Start(ComponentType space, uint64_t ms)
    {
        curspace = space;
        duration = ms * 1000000ull;
        for(Thread& t : threads)
        {
            t = Thread(Run, nullptr);
        }
        for(Thread& t : threads)
        {
            t.Join();
        }
    }
    // add task to task stack (not a queue!)
    static inline void Add(ComponentType space, const Task& task)
    {
        tasks[space].grow() = task;
    }
    // internal
    static void Run(void*)
    {
        Array<Task>& queue = tasks[curspace];
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
                if(queue.empty())
                {
                    break;
                }
                t = queue.back();
                queue.pop();
            }
            t.fn(&t);
        }
    }

    static Thread          threads[4];
    static Semaphore       sema;
    static Array<Task>     tasks[CT_Count];
    static uint64_t        duration;
    static ComponentType    curspace;
};
