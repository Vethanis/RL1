#pragma once

#include "slots.h"
#include "task.h"
#include "macro.h"

#include "sokol_time.h"

#include "resourcebase.h"

template<typename T>
struct ResourceSimple : public ResourceBase
{
    Slots<T>    m_items;

    inline int32_t Count() const final { return m_items.Count(); }
    inline void Add(slot s, int32_t, uint8_t) final
    {
        m_items.Add(s);
    }
    inline void Remove(slot s) final
    {
        m_items.Remove(s);
    }
    inline bool Exists(slot s) final
    {
        return m_items.Get(s) != nullptr;
    }
    inline void* Get(slot s) final
    {
        return m_items.Get(s);
    }
    inline void Update() final {}
    inline void CreateTasks(TaskManager&) final {}
    inline void Load(slot) final {}
    inline void Free(slot) final {}
    inline bool Init(slot) final { return true; }
    inline void Shutdown(slot) final {}
};

template<typename T>
struct Resource : public ResourceBase
{
    struct Item
    {
        ResMeta     meta;
        T           t;
    };

    typedef void (*Loader)(const ResMeta&, T&);

    Slots<Item>         m_items;
    Array<slot>         m_loadQueue;
    Array<slot>         m_freeQueue;
    Array<slot>         m_initQueue;
    Array<slot>         m_shutdownQueue;
    Array<slot>         m_removeQueue;
    uint64_t            m_timeout;

    inline int32_t Count() const final
    {
        return m_items.Count();
    }
    inline T& Get(int32_t i)
    {
        return m_items.Get(i).t;
    }
    inline const T& Get(int32_t i) const
    {
        return m_items.Get(i).t;
    }
    inline Item& GetItem(int32_t i)
    {
        return m_items.Get(i);
    }
    inline const Item& GetItem(int32_t i) const
    {
        return m_items.Get(i);
    }
    void Add(slot s, int32_t name, uint8_t loader) final
    {
        m_items.Add(s);
        Item* item = m_items.Get(s);
        memset(item, 0, sizeof(Item));
        ResMeta& meta = item->meta;
        meta.m_name = name;
        meta.m_loader = loader;
        meta.m_timestamp = stm_now();
    }
    void Remove(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
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
        return m_items.Get(s) != nullptr;
    }
    void* Get(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
            meta.m_timestamp = stm_now();

            int32_t ready = 2;
            if(meta.m_loaded == false)
            {
                m_loadQueue.grow() = s;
                --ready;
            }
            if(meta.m_initialized == false)
            {
                m_initQueue.grow() = s;
                --ready;
            }
            if(ready == 2)
            {
                return &(item->t);
            }
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

        uint64_t curTime = stm_now();
        for(int32_t i = 0; i < m_items.Count(); ++i)
        {
            Item& item = m_items.Get(i);
            if(curTime - item.meta.m_timestamp > m_timeout)
            {
                slot s = m_items.GetSlot(i);
                m_shutdownQueue.grow() = s;
                m_freeQueue.grow() = s;
            }
        }
    }
    static void TaskLoad(Task* task)
    {
        Resource* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Resource*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Load(s);
    }
    static void TaskFree(Task* task)
    {
        Resource* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Resource*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Free(s);
    }
    void CreateTasks(TaskManager& manager) final
    {
        for(slot s : m_loadQueue)
        {
            Task task;
            task.fn = TaskLoad;
            Resource* res = this;
            memcpy(task.mem + 0, &res,  sizeof(Resource*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            manager.Add(task);
        }
        for(slot s : m_freeQueue)
        {
            Task task;
            task.fn = TaskFree;
            Resource* res = this;
            memcpy(task.mem + 0, &res,  sizeof(Resource*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            manager.Add(task);
        }
        m_loadQueue.clear();
        m_freeQueue.clear();
    }
    void Load(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
            if(!meta.m_loaded)
            {
                T::Load(meta, item->t);
                meta.m_loaded = true;
            }
        }
    }
    void Free(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
            if(meta.m_loaded)
            {
                T::Free(meta, item->t);
                meta.m_loaded = false;
            }
        }
    }
    // returns whether to remove from queue
    bool Init(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
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
    void Shutdown(slot s) final
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            ResMeta& meta = item->meta;
            if(meta.m_initialized)
            {
                T::Shutdown(meta, item->t);
                meta.m_initialized = false;
            }
        }
    }
};
