#pragma once

#include "slots.h"
#include "task.h"
#include "macro.h"

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

    Slots2<T, Metadata> m_items;
    Array<Loader>       m_loaders;
    Array<slot>         m_loadQueue;
    Array<slot>         m_freeQueue;
    Array<slot>         m_initQueue;
    Array<slot>         m_shutdownQueue;
    Array<slot>         m_removeQueue;
    uint64_t            m_timeout;

    uint8_t RegisterLoader(const Loader& loader)
    {
        m_loaders.grow() = loader;
        return (uint8_t)m_loaders.count() - 1u;
    }
    slot Add(int32_t name, uint8_t loader)
    {
        slot s = m_items.Add();
        Metadata* meta = m_items.GetU(s);
        memset(meta, 0, sizeof(Metadata));
        meta->m_name = name;
        meta->m_loader = loader;
        meta->m_timestamp = stm_now();
        return s;
    }
    void Remove(slot s)
    {
        Metadata* meta = m_items.GetU(s);
        if(meta)
        {
            T* item = m_items.GetT(s);
            Loader& loader = m_loaders[meta->m_loader];
            if(meta->m_initialized)
            {
                loader.m_shutdown(*item, *meta);
            }
            if(meta->m_loaded)
            {
                loader.m_free(*item, *meta);
            }
            m_items.Remove(s);
        }
    }
    inline bool Exists(slot s)
    {
        return m_items.GetU(s) != nullptr;
    }
    T* Get(slot s)
    {
        Metadata* meta = m_items.GetU(s);
        if(meta)
        {
            meta->m_timestamp = stm_now();

            int32_t ready = 2;
            if(meta->m_loaded == false)
            {
                m_loadQueue.grow() = s;
                --ready;
            }
            if(meta->m_initialized == false)
            {
                m_initQueue.grow() = s;
                --ready;
            }
            if(ready == 2)
            {
                return m_items.GetT(s);
            }
        }
        return nullptr;
    }
    void Update()
    {
        uint64_t curTime = stm_now();
        for(int32_t i = 0; i < m_items.count(); ++i)
        {
            Metadata& meta = m_items.GetU(i);
            if(curTime - meta.m_timestamp > m_timeout)
            {
                int32_t id = m_items.m_live[i];
                int32_t gen = m_items.m_gen[id];
                slot s = { id, gen };
                m_shutdownQueue.grow() = s;
                m_freeQueue.grow() = s;
            }
        }

        for(int32_t i = m_initQueue.count() - 1; i >= 0; --i)
        {
            slot s = m_initQueue[i];
            Metadata* meta = m_slots.GetU(s);
            if(!meta)
            {
                m_initQueue.remove(i);
            }
            else if(meta->m_loaded)
            {
                m_initQueue.remove(i);
                if(meta->m_initialized == false)
                {
                    Loader& loader = m_loaders[meta->m_loader];
                    T* item = m_slots.GetT(s);
                    loader.m_init(*item, *meta);
                    meta->m_initialized = true;
                }
            }
        }

        while(!m_shutdownQueue.empty())
        {
            slot s = m_shutdownQueue.back();
            m_shutdownQueue.pop();
            Metadata* meta = m_slots.GetU(s);
            if(meta && meta->m_initialized)
            {
                Loader& loader = m_loaders[meta->m_loader];
                T* item = m_slots.GetT(s);
                loader.m_shutdown(*item, *meta);
                meta->m_initialized = false;
            }
        }
    }
    static void TaskRunner(Task* task)
    {
        Resources* res = nullptr;
        memcpy(&res, task->mem + 0, sizeof(size_t));
        res->UpdateAsync();
    }
    void CreateTasks(TaskManager& manager)
    {
        int32_t count = Max(m_loadQueue.count(), m_freeQueue.count());
        Task task;
        task.fn = TaskRunner;
        memcpy(task.mem + 0, &this, sizeof(size_t));
        for(int32_t i = 0; i < count; ++i)
        {
            manager.Add(task);
        }
    }
    void UpdateAsync()
    {
        if(!m_loadQueue.empty())
        {
            slot s = m_loadQueue.back();
            m_loadQueue.pop();
            Metadata* meta = m_slots.GetU(s);
            if(meta && !meta->m_loaded)
            {
                Loader& loader = m_loaders[meta->m_loader];
                T* item = m_slots.GetT(s);
                loader.m_load(*item, *meta);
                meta->m_loaded = true;
            }
        }        
        
        if(!m_freeQueue.empty())
        {
            slot s = m_freeQueue.back();
            m_freeQueue.pop();
            Metadata* meta = m_slots.GetU(s);
            if(meta && meta->m_loaded)
            {
                Loader& loader = m_loaders[meta->m_loader];
                T* item = m_slots.GetT(s);
                loader.m_free(*item, *meta);
                meta->m_loaded = false;
            }
        }
    }
};
