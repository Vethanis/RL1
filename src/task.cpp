#include "task.h"

#include "array.h"

#include <thread>
#include "sema.h"

std::thread     ms_threads[16];
std::mutex      ms_lock;
Semaphore       ms_startBarrier;
Semaphore       ms_finishBarrier;
Array<Task>     ms_tasks[TT_Count];
TaskType        ms_curtype;
volatile bool   ms_running;

// internal
void Run(void*)
{
    while(true)
    {
        ms_startBarrier.Wait();
        if(!ms_running)
        {
            return;
        }

        Array<Task>& queue = ms_tasks[ms_curtype];
        while(true)
        {
            Task t;
            {
                LockGuard guard(ms_lock);
                if(queue.empty())
                {
                    break;
                }
                t = queue.back();
                queue.pop();
            }
            t.fn(&t);
        }

        ms_finishBarrier.Signal();
    }
}

namespace TaskManager
{
    void Init()
    {
        ms_running = true;
        for(std::thread& t : ms_threads)
        {
            t = std::thread(Run, nullptr);
        }
    }
    void Shutdown()
    {
        ms_running = false;
        for(const std::thread& t : ms_threads)
        {
            ms_startBarrier.Signal();
        }
        for(std::thread& t : ms_threads)
        {
            t.join();
        }
    }
    void Start(TaskType space)
    {
        ms_curtype = space;
        for(const std::thread& t : ms_threads)
        {
            ms_startBarrier.Signal();
        }
        for(const std::thread& t : ms_threads)
        {
            ms_finishBarrier.Wait();
        }
    }
    void Add(TaskType space, const Task& task)
    {
        LockGuard guard(ms_lock);
        ms_tasks[space].grow() = task;
    }
};
