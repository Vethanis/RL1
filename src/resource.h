#pragma once

#include "slots.h"
#include "task.h"
#include "macro.h"
#include "component_listener.h"

#include "sokol_time.h"

template<typename T>
struct Resources
{
    struct Metadata;

    typedef void (*LoadFn)(T&, Metadata&);
    typedef void (*FreeFn)(T&, Metadata&);
    typedef void (*InitFn)(T&, Metadata&);
    typedef void (*ShutdownFn)(T&, Metadata&);

    struct Loader
    {
        LoadFn      m_load;         // load fn      (async)
        FreeFn      m_free;         // free fn      (async)
        InitFn      m_init;         // init fn      (sync)
        ShutdownFn  m_shutdown;     // shutdown fn  (sync)
    };

    struct Metadata
    {
        uint64_t    m_timestamp;            // last time touched by entity
        int32_t     m_name;                 // unique identifier, gives means to lookup path or indirect key
        uint8_t     m_loader;               // which loader to use. ie generated vs load a file
        uint8_t     m_loaded        : 1;    // is loaded        -> m_data valid
        uint8_t     m_initialized   : 1;    // is initialized   -> m_item valid
    };

    struct Item
    {
        Metadata    meta;
        T           t;
    };

    Slots<Item>         m_items;
    Array<Loader>       m_loaders;
    Array<slot>         m_loadQueue;
    Array<slot>         m_freeQueue;
    Array<slot>         m_initQueue;
    Array<slot>         m_shutdownQueue;
    Array<slot>         m_removeQueue;
    uint64_t            m_timeout;

    void Init()
    {
        EntityDestructor d;
        d.data = this;
        d.fn = ComponentDestructor;
        RegisterDestructor(d);
    }
    uint8_t RegisterLoader(const Loader& loader)
    {
        m_loaders.grow() = loader;
        return (uint8_t)m_loaders.count() - 1u;
    }
    void Add(Slot s, int32_t name, uint8_t loader)
    {
        m_items.Add(s);
        Item* item = m_items.Get(s);
        memset(item, 0, sizeof(Item));
        Metadata& meta = item->meta;
        meta.m_name = name;
        meta.m_loader = loader;
        meta.m_timestamp = stm_now();
    }
    void Remove(slot s)
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
            Loader& loader = m_loaders[meta.m_loader];
            if(meta.m_initialized)
            {
                loader.m_shutdown(item->t, meta);
            }
            if(meta.m_loaded)
            {
                loader.m_free(item->t, meta);
            }
            m_items.Remove(s);
        }
    }
    inline bool Exists(slot s)
    {
        return m_items.Get(s) != nullptr;
    }
    T* Get(slot s)
    {
        Item* item = m_items.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
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
                return item;
            }
        }
        return nullptr;
    }
    void Update()
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
        Resources* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Resources*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Load(s);
    }
    static void TaskFree(Task* task)
    {
        Resources* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(Resources*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Free(s);
    }
    static void ComponentDestructor(void* data, slot s)
    {
        Resources* res = (Resources*)data;
        res->Remove(s);
    }
    void CreateTasks(TaskManager& manager)
    {
        for(slot s : m_loadQueue)
        {
            Task task;
            task.fn = TaskLoad;
            memcpy(task.mem + 0, &this, sizeof(Resources*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            manager.Add(task);
        }
        for(slot s : m_freeQueue)
        {
            Task task;
            task.fn = TaskFree;
            memcpy(task.mem + 0, &this, sizeof(Resources*));
            memcpy(task.mem + 1, &s,    sizeof(slot));
            manager.Add(task);
        }
        m_loadQueue.clear();
        m_freeQueue.clear();
    }
    void Load(slot s)
    {
        Item* item = m_slots.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
            if(!meta.m_loaded)
            {
                Loader& loader = m_loaders[meta.m_loader];
                loader.m_load(item->t, meta);
                meta.m_loaded = true;
            }
        }
    }
    void Free(slot s)
    {
        Item* item = m_slots.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
            if(meta.m_loaded)
            {
                Loader& loader = m_loaders[meta.m_loader];
                loader.m_free(item->t, meta);
                meta.m_loaded = false;
            }
        }
    }
    // returns whether to remove from queue
    bool Init(slot s)
    {
        Item* item = m_slots.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
            if(meta.m_loaded)
            {
                if(!meta.m_initialized)
                {
                    Loader& loader = m_loaders[meta.m_loader];
                    loader.m_init(item->t, meta);
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
        Item* item = m_slots.Get(s);
        if(item)
        {
            Metadata& meta = item->meta;
            if(meta.m_initialized)
            {
                Loader& loader = m_loaders[meta.m_loader];
                loader.m_shutdown(item->t, meta);
                meta.m_initialized = false;
            }
        }
    }
};
