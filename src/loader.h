
#include "resource.h"
#include "slots.h"
#include "task.h"
#include "macro.h"

#include "sokol_time.h"

template<typename T>
struct Loader : public LoaderBase
{
    struct Item
    {
        LoaderMeta  meta;
        T           t;
    };

    Slots<Item>         m_items;
    Array<slot>         m_loadQueue;
    Array<slot>         m_freeQueue;
    Array<slot>         m_initQueue;
    Array<slot>         m_shutdownQueue;
    uint64_t            m_taskDuration;

    inline void Add(slot s) final 
    {
        Assert(false);
    }
    void Add(slot s, int32_t name, uint8_t method) final
    {
        m_items.Add(s);
        Item* item = m_items.Get(s);
        memset(item, 0, sizeof(Item));
        LoaderMeta& meta = item->meta;
        meta.m_name = name;
        meta.m_method = method;
        meta.m_timestamp = stm_now();
    }
    void Remove(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            if(meta.m_initialized)
            {
                T::Shutdown(meta, item->t);
            }
            if(meta.m_loaded)
            {
                T::Free(meta, item->t);
            }
            m_items.Remove(s);
        }
    }
    inline bool Exists(slot s) final
    {
        return m_items.Exists(s);
    }
    void* Get(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            meta.m_timestamp = stm_now();

            if(meta.m_loaded == false)
            {
                m_loadQueue.grow() = s;
                return nullptr;
            }
            if(meta.m_initialized == false)
            {
                m_initQueue.grow() = s;
                return nullptr;
            }
            return &(item->t);
        }
        return nullptr;
    }
    void Update() final
    {
        for(int32_t i = m_initQueue.count() - 1; i >= 0; --i)
        {
            slot s = m_initQueue[i];
            if(Init(s))
            {
                m_initQueue.remove(i);
            }
        }

        while(!m_shutdownQueue.empty())
        {
            slot s = m_shutdownQueue.back();
            m_shutdownQueue.pop();
            Shutdown(s);
        }
    }
    void DoTasks() final
    {
        for(slot s : m_loadQueue)
        {
            Task task;
            task.fn = TaskLoad;
            Loader* res = this;
            memcpy(task.mem + 0, &res,  sizeof(Loader*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            TaskManager::Add(T::ms_type, task);
        }
        for(slot s : m_freeQueue)
        {
            Task task;
            task.fn = TaskFree;
            Loader* res = this;
            memcpy(task.mem + 0, &res,  sizeof(Loader*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            TaskManager::Add(T::ms_type, task);
        }
        m_loadQueue.clear();
        m_freeQueue.clear();
        TaskManager::Start(T::ms_type, m_taskDuration);
    }
    void Load(slot s) 
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            if(!meta.m_loaded)
            {
                T::Load(meta, item->t);
                meta.m_loaded = true;
            }
        }
    }
    void Free(slot s) 
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            if(meta.m_loaded)
            {
                T::Free(meta, item->t);
                meta.m_loaded = false;
            }
        }
    }
    // returns whether to remove from queue
    bool Init(slot s) 
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            if(meta.m_loaded)
            {
                if(!meta.m_initialized)
                {
                    T::Init(meta, item->t);
                    meta.m_initialized = true;
                }
                return true;
            }
            // not loaded yet, keep in queue
            return false;
        }
        return true;
    }
    void Shutdown(slot s) 
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            LoaderMeta& meta = item->meta;
            if(meta.m_initialized)
            {
                T::Shutdown(meta, item->t);
                meta.m_initialized = false;
            }
        }
    }
    static void TaskLoad(Task* task)
    {
        Loader* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Loader*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Load(s);
    }
    static void TaskFree(Task* task)
    {
        Loader* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Loader*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Free(s);
    }
};
