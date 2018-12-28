#pragma once

#include "slot.h"

enum ComponentType
{
    CT_Render = 0,
    CT_Physics,
    CT_Children,
    CT_Pathfind,
    CT_AI,
    CT_Weapon,
    CT_Health,
    CT_Control,
    CT_Inventory,
    CT_Count  
};

namespace Components
{
    void Init();
    slot Create();
    void Destroy(slot s);
    void* Get(ComponentType type, slot s);
    const void* GetConst(ComponentType type, slot s);
    void Add(ComponentType type, slot s);
    void Remove(ComponentType type, slot s);
    bool Exists(slot s);
    bool Has(ComponentType type, slot s);
    void* GetAdd(ComponentType type, slot s);
    
    const slot* begin();
    const slot* end();

    template<typename T>
    inline T* Get(slot s)
    {
        return static_cast<T*>(Get(T::ms_type, s));
    }
    template<typename T>
    inline const T* GetConst(slot s)
    {
        return static_cast<T*>(GetConst(T::ms_type, s));
    }
    template<typename T>
    inline void Add(slot s)
    {
        Add(T::ms_type, s);
    }
    template<typename T>
    inline void Remove(slot s)
    {
        Remove(T::ms_type, s);
    }
    template<typename T>
    inline bool Has(slot s)
    {
        return Has(T::ms_type, s);
    }
    template<typename T>
    inline T* GetAdd(slot s)
    {
        return static_cast<T*>(GetAdd(T::ms_type, s));
    }
};

