#include "ecs.h"

#include "gen_array.h"
#include "bitfield.h"
#include "find.h"

#include "component_funcs.h"

namespace ECS
{
    static constexpr Allocator ms_alloc =
    {
        Malloc::Allocate,
        Malloc::Reallocate,
        Malloc::Free,
        4096u,
    };

    static GenIndices<ms_alloc>     ms_indices;                 // active slots
    static Slice<ComponentFlags>    ms_flags;                   // what components exist, per-entity
    static ComponentData            ms_components[CT_Count];    // component allocations as bytes
    static u32                      ms_capacity;                // capacity of component arrays

    static ComponentData GetComponentData(u32 e, u32 c)
    {
        return ms_components[c].opaque(e, sc_ComponentSize[c]);
    }

    void Init()
    {

    }
    void Update()
    {

    }
    void Shutdown()
    {
        const u32 cap = ms_indices.capacity();
        for(u32 c = 0u; c < CT_Count; ++c)
        {
            for(u32 e = 0u; e < cap; ++e)
            {
                if(ms_flags[e].has(c))
                {
                    sc_ComponentDrop[c](GetComponentData(e, c));
                }
            }
            ms_alloc.Free(ms_components[c]);
        }
        
        ms_alloc.Free(ms_flags);
        ms_indices.reset();
        ms_capacity = 0u;
    }

    Entity Create()
    {
        const Slot s = ms_indices.create();
        const u32 newCap = s.id + 1u;

        if (newCap > ms_capacity)
        {
            ms_capacity = newCap;
            ms_alloc.Realloc(ms_flags, newCap);

            for(u32 i = 0; i < CT_Count; ++i)
            {
                ms_alloc.Realloc(ms_components[i], newCap * sc_ComponentSize[i]);
            }
        }

        for(u32 c = 0u; c < CT_Count; ++c)
        {
            EraseComp(GetComponentData(s.id, c));
        }
        EraseR(ms_flags[s.id]);

        return { s };
    }

    bool Destroy(Entity e)
    {
        if (ms_indices.destroy(e.s))
        {
            const u32 id = e.s.id;
            for(u32 c = 0u; c < CT_Count; ++c)
            {
                if(ms_flags[id].has(c))
                {
                    sc_ComponentDrop[c](GetComponentData(id, c));
                }
            }
            EraseR(ms_flags[id]);

            return true;
        }
        return false;
    }

    bool Exists(Entity e)
    {
        return ms_indices.exists(e.s);
    }

    Slice<const ComponentFlags> GetFlags()
    {
        return ms_flags.subslice(0, ms_capacity).to_const();
    }

    bool AddComponent(Entity e, ComponentType type)
    {
        const u32 id = e.s.id;
        if(Exists(e) && !ms_flags[id].has(type))
        {
            sc_ComponentNew[type](GetComponentData(id, type));
            ms_flags[id].set(type);
            return true;
        }
        return false;
    }

    bool RemoveComponent(Entity e, ComponentType type)
    {
        const u32 id = e.s.id;
        if(Exists(e) && ms_flags[id].has(type))
        {
            sc_ComponentDrop[type](GetComponentData(id, type));
            ms_flags[id].unset(type);
            return true;
        }
        return false;
    }

    ComponentData GetComponent(Entity e, ComponentType type)
    {
        const u32 id = e.s.id;
        if(Exists(e) && ms_flags[id].has(type))
        {
            return GetComponentData(id, type);
        }
        return { 0, 0 };
    }

    ComponentData GetAllComponents(ComponentType type)
    {
        return ms_components[type].subslice(0, ms_indices.capacity() * sc_ComponentSize[type]);
    }
};
