#pragma once

#include "macro.h"
#include "component_types_thin.h"
#include "slice.h"

namespace ECS
{
    void Init();
    void Update();
    void Shutdown();

    Entity Create();
    bool Destroy(Entity e);
    bool Exists(Entity e);

    Slice<const ComponentFlags> GetFlags();

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
