#pragma once

#include "templates.h"
#include "component_types_thin.h"

namespace ECS
{
    void Init();
    void Update();
    void Shutdown();

    Entity Create();
    bool Destroy(Entity e);
    bool Exists(Entity e);

    Slice<const ComponentFlags> GetFlags();
    CDwordSlice GetGens();

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
        return Cast<T>(GetComponent(e, GetComponentType<T>()));
    }

    template<typename T>
    inline Slice<T> GetAll()
    {
        let data = GetAllComponents(GetComponentType<T>());
        return { Cast<T>(data), data.bytes() / sizeof(T) };
    }

    bool& ImVisEnabled();
    void ImVisUpdate();
};
