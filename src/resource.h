#pragma once

#include "slot.h"

struct ResourceBase
{
    virtual bool IsLoader() const { return false; };
    virtual void Add(slot s)        = 0;
    virtual void Remove(slot s)     = 0;
    virtual bool Exists(slot s)     = 0;
    virtual void* Get(slot s)       = 0;
};

struct LoaderMeta
{
    uint64_t    m_timestamp;            // last time touched by entity
    int32_t     m_name;                 // unique identifier, gives means to lookup path or indirect key
    uint8_t     m_method;               // which loader to use. ie generated vs load a file
    uint8_t     m_loaded        : 1;    // is loaded        -> m_data valid
    uint8_t     m_initialized   : 1;    // is initialized   -> m_item valid
};

struct LoaderBase : public ResourceBase
{
    inline bool IsLoader() const final { return true; }
    virtual void Add(slot s, int32_t name, uint8_t method) = 0;
    virtual void Update() = 0;
    virtual void DoTasks() = 0;
};
