#include "task.h"

#include "array.h"
#include "thread.h"
#include "sema.h"
#include "sokol_time.h"

Thread          ms_threads[4];
Semaphore       ms_sema;
Array<Task>     ms_tasks[RT_Count];
uint64_t        ms_duration;
ResourceType    ms_curtype;

// internal
void Run(void*)
{
    Array<Task>& queue = ms_tasks[ms_curtype];
    uint64_t started = stm_now();
    while(true)
    {
        uint64_t now = stm_now();
        if(now - started >= ms_duration)
        {
            break;
        }

        Task t;
        {
            LockGuard guard(ms_sema);
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

namespace TaskManager
{
    // will use up to ms milliseconds to do parallel updates; can early out
    // begins multithreaded phase of engine frame; blocking
    void Start(ResourceType space, uint64_t ms)
    {
        ms_curtype = space;
        ms_duration = ms * 1000000ull;
        for(Thread& t : ms_threads)
        {
            t = Thread(Run, nullptr);
        }
        for(Thread& t : ms_threads)
        {
            t.Join();
        }
    }
    // add task to task stack (not a queue!)
    void Add(ResourceType space, const Task& task)
    {
        ms_tasks[space].grow() = task;
    }
};
