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
    void Start(ResourceType space, uint64_t ms)
    {
        ms_curtype = space;
        ms_duration = ms * 1000000ull;
        for(Thread& t : ms_threads)
        {
            // need to make this re-use threads instead of launching new ones
            t = Thread(Run, nullptr);
        }
        for(Thread& t : ms_threads)
        {
            t.Join();
        }
    }
    void Add(ResourceType space, const Task& task)
    {
        ms_tasks[space].grow() = task;
    }
};
