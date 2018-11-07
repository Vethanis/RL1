#pragma once

#include "slot.h"
#include "resource.h"

struct Loader
{
    virtual ~Loader(){}
    virtual slot Add() = 0;
    virtual void IncRef(slot s) = 0;
    virtual void DecRef(slot s) = 0;
    virtual Resource* Get(slot s) = 0;
    virtual bool Exists(slot s) = 0;
    virtual void Update() = 0;
    virtual void DoTasks(uint64_t ms) = 0;
};

namespace Loaders
{
    void Init();
    slot Add(ResourceType type);
    void IncRef(ResourceType type, slot s);
    void DecRef(ResourceType type, slot s);
    Resource* Get(ResourceType type, slot s);
    bool Exists(ResourceType type, slot s);
    void Update(ResourceType type);
    void DoTasks(ResourceType type, uint64_t ms);

    template<typename T>
    inline slot Add() { return Add(T::ms_type); }
    template<typename T>
    inline void IncRef(slot s) { IncRef(T::ms_type, s); }
    template<typename T>
    inline void DecRef(slot s) { DecRef(T::ms_type, s); }
    template<typename T>
    inline T* Get(slot s) { return static_cast<T*>(Get(T::ms_type, s)); }
    template<typename T>
    inline bool Exists(slot s) { return Exists(T::ms_type, s); }
    template<typename T>
    inline void Update() { Update(T::ms_type); }
    template<typename T>
    inline void DoTasks(uint64_t ms) { DoTasks(T::ms_type, ms); }
};
