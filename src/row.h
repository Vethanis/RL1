#pragma once

#include "slot.h"
#include "component.h"

struct Row
{
    uint32_t    m_gen;
    Component*  m_components[CT_Count];
};

namespace Rows
{
    void Init();
    slot Create();
    void Destroy(slot s);
    Component* Get(ComponentType type, slot s);
    template<typename T>
    inline T* Get(slot s)
    {
        return static_cast<T*>(Get(T::ms_type, s));
    }
    void Add(ComponentType type, slot s);
    template<typename T>
    inline void Add(slot s)
    {
        Add(T::ms_type, s);
    }
    void Remove(ComponentType type, slot s);
    template<typename T>
    inline void Remove(slot s)
    {
        Remove(T::ms_type, s);
    }
    bool Exists(slot s);
    bool Has(ComponentType type, slot s);
    template<typename T>
    inline bool Has(slot s)
    {
        return Has(T::ms_type, s);
    }
    inline Component* GetAdd(ComponentType type, slot s);
    template<typename T>
    inline T* GetAdd(slot s)
    {
        return static_cast<T*>(GetAdd(T::ms_type, s));
    }
};
