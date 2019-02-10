#include "task.h"

#include "array.h"

#include <thread>
#include "sema.h"

constexpr int32_t NumThreads = 16;

static std::thread     ms_threads[NumThreads];
static Array<Task>     ms_queues[NumThreads];
static std::mutex      ms_lock;
static Semaphore       ms_startBarrier;
static Semaphore       ms_finishBarrier;
static Array<Task>     ms_tasks[TT_Count];
static TaskType        ms_curtype;
static volatile bool   ms_running = false;
static int32_t         ms_granularity = 16;

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
        for(int32_t i = 0; i < NumThreads; ++i)
        {
            ms_threads[i] = std::thread(Run, i);
        }
    }
    void Shutdown()
    {
        ms_running = false;
        ms_startBarrier.Signal(NumThreads);
        for(std::thread& t : ms_threads)
        {
            t.join();
        }
    }
    void Start(TaskType space, int32_t granularity)
    {
        ms_curtype = space;
        ms_granularity = granularity;
        ms_startBarrier.Signal(NumThreads);
        ms_finishBarrier.Wait(NumThreads);
    }
    void Add(TaskType space, const Task& task)
    {
        ms_tasks[space].grow() = task;
    }
};
