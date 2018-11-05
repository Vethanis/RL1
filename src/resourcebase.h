#pragma once

#include <stdint.h>

#include "slot.h"

struct TaskManager;

struct ResMeta
{
    uint64_t    m_timestamp;            // last time touched by entity
    int32_t     m_name;                 // unique identifier, gives means to lookup path or indirect key
    uint8_t     m_loader;               // which loader to use. ie generated vs load a file
    uint8_t     m_loaded        : 1;    // is loaded        -> m_data valid
    uint8_t     m_initialized   : 1;    // is initialized   -> m_item valid
};

enum ResourceType
{
    RT_Buffer = 0,
    RT_Image,
    RT_Transform,
    RT_Count
};

struct ResourceBase
{
    virtual void Add(slot s, int32_t name, uint8_t loader) = 0;
    virtual void Remove(slot s) = 0;
    virtual void* Get(slot s) = 0;
    virtual bool Exists(slot s) = 0;
    virtual void Update() = 0;
    virtual int32_t Count() const = 0;
    virtual void CreateTasks(TaskManager& manager) = 0;
    virtual bool Init(slot s) = 0;
    virtual void Shutdown(slot s) = 0;
    virtual void Load(slot s) = 0;
    virtual void Free(slot s) = 0;

    static void Setup();
    static void UpdateAll()
    {
        for(ResourceBase* res : s_resources)
        {
            res->Update();
        }
    }
    static void TaskAll(TaskManager& manager);
    static void RemoveAll(slot s)
    {
        for(ResourceBase* res : s_resources)
        {
            res->Remove(s);
        }
    }
    inline static void Set(ResourceType rt, ResourceBase* res)
    {
        s_resources[rt] = res;
    }
    inline static void SetDuration(ResourceType rt, uint64_t ms)
    {
        s_durations[rt] = ms;
    }
    inline static ResourceBase* Get(ResourceType rt)
    {
        return s_resources[rt];
    }

    static ResourceBase* s_resources[RT_Count];
    static uint64_t      s_durations[RT_Count];
};
