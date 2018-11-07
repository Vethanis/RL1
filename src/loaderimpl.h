#pragma once

#include <string.h>
#include <new>

#include "loader.h"
#include "array.h"
#include "task.h"

template<typename T>
struct LoaderImpl : public Loader
{
    struct Item
    {
        uint32_t    m_gen;
        int32_t     m_refs;                 // ref count
        uint8_t     m_loaded        : 1;    // is loaded        -> m_data valid
        uint8_t     m_initialized   : 1;    // is initialized   -> m_item valid
        T           t;
    };

    Array<Item>         m_items;
    Array<int32_t>      m_free;
    Array<slot>         m_loadQueue;
    Array<slot>         m_freeQueue;
    Array<slot>         m_initQueue;
    Array<T>            m_shutdownQueue;

    inline bool Exists(slot s) final 
    {
        return s.id < (uint32_t)m_items.count() && s.gen == m_items[s.id].m_gen;
    }
    slot Add() final
    {
        if(m_free.empty())
        {
            m_free.grow() = m_items.count();
            memset(&m_items.grow(), 0, sizeof(Item));
        }
        slot s;
        s.id = m_free.back();
        m_free.pop();
        Item& item = m_items[s.id];
        new (&item) Item();
        s.gen = item.m_gen;
        item.m_refs = 1;
        return s;
    }
    void IncRef(slot s) final 
    {
        if(!Exists(s))
        {
            return;
        }
        Item& item = m_items[s.id];
        item.m_refs++;
    }
    void DecRef(slot s) final
    {
        if(!Exists(s))
        {
            return;
        }
        Item& item = m_items[s.id];
        --item.m_refs;

        if(item.m_refs == 0)
        {
            item.m_gen++;
            if(item.m_initialized)
            {
                memcpy(&m_shutdownQueue.grow(), &item.t, sizeof(T));
                item.m_initialized = false;
            }
            if(item.m_loaded)
            {
                item.t.Free();
                item.m_loaded = false;
            }
            (&item)->~Item();
            m_free.grow() = s.id;
        }
    }
    Resource* Get(slot s) final
    {
        if(!Exists(s))
        {
            return nullptr;
        }

        Item& item = m_items[s.id];

        bool ready = true;
        if(!item.m_loaded)
        {
            m_loadQueue.grow() = s;
            ready = false;
        }
        if(!item.m_initialized)
        {
            m_initQueue.grow() = s;
            ready = false;
        }

        return ready ? &item.t : nullptr;
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
            m_shutdownQueue.back().Shutdown();
            m_shutdownQueue.pop();
        }
    }
    void DoTasks(uint64_t ms) final
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
        TaskManager::Start(T::ms_type, ms);
    }
    void Load(slot s) 
    {
        if(!Exists(s))
        {
            return;
        }
        Item& item = m_items[s.id];
        if(!item.m_loaded)
        {
            item.t.Load();
            item.m_loaded = true;
        }
    }
    void Free(slot s) 
    {
        if(!Exists(s))
        {
            return;
        }
        Item& item = m_items[s.id];
        if(item.m_loaded)
        {
            item.t.Free();
            item.m_loaded = false;
        }
    }
    bool Init(slot s) 
    {
        if(!Exists(s))
        {
            return false;
        }
        Item& item = m_items[s.id];
        if(item.m_loaded)
        {
            if(!item.m_initialized)
            {
                item.t.Init();
                item.m_initialized = true;
            }
            return true;
        }
        return false;
    }
    static void TaskLoad(Task* task)
    {
        LoaderImpl* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(LoaderImpl*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Load(s);
    }
    static void TaskFree(Task* task)
    {
        LoaderImpl* res = nullptr;
        slot s = {0};
        memcpy(&res, task->mem + 0, sizeof(LoaderImpl*));
        memcpy(&s,   task->mem + 1, sizeof(slot));
        res->Free(s);
    }
};
