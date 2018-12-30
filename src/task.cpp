#include "task.h"

#include "array.h"

#include <thread>
#include "sema.h"

constexpr int32_t NumThreads = 16;

std::thread     ms_threads[NumThreads];
Array<Task>     ms_queues[NumThreads];
std::mutex      ms_lock;
Semaphore       ms_startBarrier;
Semaphore       ms_finishBarrier;
Array<Task>     ms_tasks[TT_Count];
TaskType        ms_curtype;
volatile bool   ms_running = false;
bool            ms_working = false;
int32_t         ms_granularity = 4;

// internal
void Run(int32_t tid)
{
    while(true)
    {
        ms_startBarrier.Wait();
        if(!ms_running)
        {
            return;
        }

        Array<Task>& queue = ms_tasks[ms_curtype];
        Array<Task>& local = ms_queues[tid];
        while(true)
        {
            {
                LockGuard guard(ms_lock);
                for(int32_t i = 0; i < ms_granularity; ++i)
                {
                    if(queue.empty())
                    {
                        break;
                    }
                    memcpy(&local.grow(), &queue.back(), sizeof(Task));
                    queue.pop();
                }
            }
            if(local.empty())
            {
                break;
            }
            for(Task& t : local)
            {
                t.fn(&t);
            }
            local.clear();
        }

        ms_finishBarrier.Signal();
    }
}

namespace TaskManager
{
    void Init()
    {
        ms_running = true;
        for(int32_t tid = 0; tid < NumThreads; ++tid)
        {
            ms_threads[tid] = std::thread(Run, tid);
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
    void Start(TaskType space, int32_t granularity)
    {
        ms_curtype = space;
        ms_working = true;
        ms_granularity = granularity;
        for(const std::thread& t : ms_threads)
        {
            ms_startBarrier.Signal();
        }
        for(const std::thread& t : ms_threads)
        {
            ms_finishBarrier.Wait();
        }
        ms_working = false;
    }
    void Add(TaskType space, const Task& task)
    {
        Assert(!ms_working);
        ms_tasks[space].grow() = task;
    }
};
