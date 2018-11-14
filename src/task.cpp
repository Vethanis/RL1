#include "task.h"

#include "array.h"
#include "thread.h"
#include "sema.h"
#include "sokol_time.h"

Thread          ms_threads[8];
Semaphore       ms_sema;
Semaphore       ms_barrier;
Array<Task>     ms_tasks[TT_Count];
uint64_t        ms_duration;
TaskType        ms_curtype;
volatile bool   ms_running;

// internal
void Run(void*)
{
    while(true)
    {
        ms_barrier.Wait();
        if(!ms_running)
        {
            return;
        }

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
}

namespace TaskManager
{
    void Init()
    {
        ms_running = true;
        for(Thread& t : ms_threads)
        {
            t = Thread(Run, nullptr);
        }
    }
    void Shutdown()
    {
        ms_running = false;
        for(const Thread& t : ms_threads)
        {
            ms_barrier.Signal();
        }
        for(Thread& t : ms_threads)
        {
            t.Join();
        }
    }
    void Start(TaskType space, uint64_t ms)
    {
        ms_curtype = space;
        ms_duration = ms * 1000000ull;
        for(const Thread& t : ms_threads)
        {
            ms_barrier.Signal();
        }
    }
    void Add(TaskType space, const Task& task)
    {
        ms_tasks[space].grow() = task;
    }
};
