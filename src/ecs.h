#pragma once

#include "slot.h"
#include "slice.h"
#include "component_types.h"

struct Entity
{
    Slot s;
};

namespace ECS
{
    void Init();
    void Shutdown();

    Entity Create();
    bool Destroy(Entity e);
    bool Exists(Entity e);

    // find the number of entities with the given component types
    // optionally get the entities with the given component types
    u32 Query(Slice<const ComponentType> query, Entity* pOptionalOut);

    bool AddComponent(Entity e, ComponentType type);
    bool RemoveComponent(Entity e, ComponentType type);
    ComponentData GetComponent(Entity e, ComponentType type);
    ComponentData GetAllComponents(ComponentType type);

    template<typename T>
    inline bool Add(Entity e)
    {
        return AddComponent(e, GetComponentType<T>());
    }
    
    template<typename T>
    inline bool Remove(Entity e)
    {
        return RemoveComponent(e, GetComponentType<T>());
    }

    template<typename T>
    inline T* Get(Entity e)
    {
        return ToType<T>(GetComponent(e, GetComponentType<T>())).begin();
    }

    template<typename T>
    inline Slice<T> GetAll()
    {
        return ToType<T>(GetAllComponents(GetComponentType<T>()));
    }
};
