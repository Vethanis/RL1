// ****************************************************************************
// Copyright (c) 2019 Lauren Kara Hughes                                      *
// This file is licensed under the terms of the Apache version 2.0 license    *
// https://www.apache.org/licenses/LICENSE-2.0.txt                            *
// ****************************************************************************

#pragma once

#include <stdint.h>

typedef struct Task
{
    void(*func)(struct Task*);
    uint8_t data[32];
} Task;

typedef struct TaskMgrDesc
{
    uint32_t numThreads;    // number of threads to pool
    uint32_t granularity;   // number of tasks per thread
} TaskMgrDesc;

typedef struct TaskSubmitDesc
{
    Task* tasks;        // task array
    uint32_t numTasks;  // number of tasks to submit
} TaskSubmitDesc;

void TaskMgrInit(const TaskMgrDesc* desc);
void TaskMgrShutdown();
void TaskMgrSubmit(TaskSubmitDesc* desc);

#if TASK_IMPL

    #include <stdlib.h>
    #include <string.h>
    #include <signal.h>

    #define SIGINT_ASSERT(x) { if(!(x)) { raise(SIGINT); } }

    #ifndef TASK_ASSERT
        #define TASK_ASSERT SIGINT_ASSERT
    #endif // TASK_ASSERT

// ----------------------------------------------------------------------------

    typedef struct Queue
    {
        Task*    tasks;
        uint32_t count;
        uint32_t cap;
    } Queue;

    typedef struct TaskMgr
    {
        void*    beginSem;
        void*    endSem;
        void*    queueSem;
        void*    threads;
        Queue    queue;
        uint32_t numThreads;
        uint32_t granularity;
        volatile uint32_t running;
    } TaskMgr;

    static TaskMgr ms_mgr;

// ----------------------------------------------------------------------------

    static void ThreadFn();
    static void QueuePush(
        Queue*      queue,
        const Task* tasks,
        uint32_t    numTasks);
    static uint32_t QueuePop(
        Queue*      queue,
        uint32_t    granularity,
        Task*       tasks);

    static uint32_t DivCeil(uint32_t nom, uint32_t denom)
    {
        return (nom + (denom - 1)) / denom;
    }

// ----------------------------------------------------------------------------

    static void ImplSignal(void* sem);
    static void ImplWait(void* sem);
    static void ImplInit(const TaskMgrDesc* desc);
    static void ImplShutdown();

// ----------------------------------------------------------------------------

    void TaskMgrInit(const TaskMgrDesc* desc)
    {
        TASK_ASSERT(desc);

        memset(&ms_mgr, 0, sizeof(TaskMgr));

        ImplInit(desc);

        TASK_ASSERT(ms_mgr.beginSem);
        TASK_ASSERT(ms_mgr.endSem);
        TASK_ASSERT(ms_mgr.queueSem);
        TASK_ASSERT(ms_mgr.threads);

        ms_mgr.numThreads = desc->numThreads;
        ms_mgr.granularity = desc->granularity;
        ms_mgr.running = 1;
    }

    void TaskMgrShutdown()
    {
        ms_mgr.running = 0;

        ImplShutdown();

        free(ms_mgr.threads);
        free(ms_mgr.queue.tasks);

        memset(&ms_mgr, 0, sizeof(ms_mgr));
    }

    void TaskMgrSubmit(TaskSubmitDesc* desc)
    {
        ImplWait(ms_mgr.queueSem);
        {
            QueuePush(&ms_mgr.queue, desc->tasks, desc->numTasks);
        }
        ImplSignal(ms_mgr.queueSem);

        const uint32_t numSigs = DivCeil(desc->numTasks, ms_mgr.granularity);

        for(uint32_t i = 0; i < numSigs; ++i)
        {
            ImplSignal(ms_mgr.beginSem);
        }

        for(uint32_t i = 0; i < numSigs; ++i)
        {
            ImplWait(ms_mgr.endSem);
        }
    }

// ----------------------------------------------------------------------------

    static void ThreadFn()
    {
        const uint32_t gran = ms_mgr.granularity;
        Task* tasks = (Task*)malloc(sizeof(Task) * gran);

        while(1)
        {
            ImplWait(ms_mgr.beginSem);
            {
                if(!ms_mgr.running)
                {
                    break;
                }

                uint32_t len = 0;

                ImplWait(ms_mgr.queueSem);
                {
                    len = QueuePop(&ms_mgr.queue, gran, tasks);
                }
                ImplSignal(ms_mgr.queueSem);

                for(uint32_t i = 0; i < len; ++i)
                {
                    tasks[i].func(tasks + i);
                }
            }
            ImplSignal(ms_mgr.endSem);
        }

        free(tasks);
    }

// ----------------------------------------------------------------------------

    static void QueuePush(
        Queue*      queue,
        const Task* tasks,
        uint32_t    numTasks)
    {
        uint32_t ct = queue->count;
        uint32_t cap = queue->cap;
        const uint32_t newCt = ct + numTasks;

        if(newCt >= cap)
        {
            cap *= 2;
            cap = cap > newCt ? cap : newCt;

            queue->cap = cap;
            queue->tasks = (Task*)realloc(queue->tasks, sizeof(Task) * cap);
        }

        memcpy(queue->tasks + ct, tasks, sizeof(Task) * numTasks);
        queue->count = newCt;
    }

// ----------------------------------------------------------------------------

    static uint32_t QueuePop(
        Queue*      queue,
        uint32_t    granularity,
        Task*       tasks)
    {
        uint32_t ct = queue->count;
        uint32_t len = granularity < ct ? granularity : ct;

        ct -= len;
        queue->count = ct;

        memcpy(tasks, ms_mgr.queue.tasks + ct, sizeof(Task) * len);

        return len;
    }

// ----------------------------------------------------------------------------

#if _MSVC_VER
    #error no winthreads impl yet
#else

    #include <pthread.h>
    #include <semaphore.h>

    static sem_t        ms_beginSem;
    static sem_t        ms_endSem;
    static sem_t        ms_queueSem;
    static pthread_t*   ms_threads;

    static void ImplSignal(void* sem)
    {
        sem_post((sem_t*)sem);
    }
    static void ImplWait(void* sem)
    {
        sem_wait((sem_t*)sem);
    }
    static void* ImplThreadFn(void* userdata)
    {
        ThreadFn();
        pthread_exit(NULL);
        return NULL;
    }
    static void ImplInit(const TaskMgrDesc* desc)
    {
        int32_t rv = 0;

        rv += sem_init(&ms_beginSem, 0, 0);
        rv += sem_init(&ms_endSem, 0, 0);
        rv += sem_init(&ms_queueSem, 0, 1);
        ms_mgr.beginSem   = &ms_beginSem;
        ms_mgr.endSem     = &ms_endSem;
        ms_mgr.queueSem   = &ms_queueSem;

        TASK_ASSERT(rv == 0);

        ms_threads = (pthread_t*)malloc(sizeof(pthread_t) * desc->numThreads);
        ms_mgr.threads = ms_threads;

        TASK_ASSERT(ms_threads);

        for(uint32_t i = 0; i < desc->numThreads; ++i)
        {
            rv += pthread_create(ms_threads + i, NULL, ImplThreadFn, NULL);
        }

        TASK_ASSERT(rv == 0);
    }
    static void ImplShutdown()
    {
        int32_t rv = 0;

        for(uint32_t i = 0; i < ms_mgr.numThreads; ++i)
        {
            rv += sem_post(&ms_beginSem);
        }

        TASK_ASSERT(rv == 0);

        for(uint32_t i = 0; i < ms_mgr.numThreads; ++i)
        {
            rv += pthread_join(ms_threads[i], NULL);
        }

        TASK_ASSERT(rv == 0);

        rv += sem_destroy(&ms_queueSem);
        rv += sem_destroy(&ms_endSem);
        rv += sem_destroy(&ms_beginSem);

        TASK_ASSERT(rv == 0);
    }
#endif // _MSVC_VER

#endif // TASK_IMPL
