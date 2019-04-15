#include "ecs.h"

#include "gen_array.h"
#include "bitfield.h"
#include "find.h"

#include "component_types.h"
#include "component_funcs.h"

// ----------------------------------------------------------------------------

using ComponentFlags = BitField<CT_Count>;

static constexpr Allocator ms_alloc =
{
    Malloc::Allocate,
    Malloc::Reallocate,
    Malloc::Free,
    4096u,
};

static GenIndices<ms_alloc>                     ms_indices;                     // active slots
static Slice<ComponentFlags>                    ms_flags;                       // what components exist, per-entity
static ComponentData                            ms_components[CT_Count];        // component allocations as bytes
static u32                                      ms_counts[CT_Count];            // how many components exist
static u32                                      ms_capacity;                    // capacity of component arrays

// ----------------------------------------------------------------------------

static ComponentData GetComponentData(u32 e, u32 c)
{
    return ms_components[c].opaque(e, sc_ComponentSize[c]);
}

// ----------------------------------------------------------------------------

namespace ECS
{
    void Init()
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
        EraseA(ms_counts);
        ms_capacity = 0u;
    }

    Entity Create()
    {
        const Slot s     = ms_indices.create();
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

    u32 Query(Slice<const ComponentType> query, Entity* pOut)
    {
        ComponentFlags qflags;
        EraseR(qflags);
        qflags.set(ToDwords(query));

        CDwordSlice gens = ms_indices.gens();
        u32 count = 0;

        if(pOut)
        {
            for(u32 e = 0; e < gens.size(); ++e)
            {
                if((gens[e] & 1u) && ms_flags[e].has_all(qflags))
                {
                    Slot s;
                    s.id  = e;
                    s.gen = gens[e];

                    pOut[count] = { s };
                    ++count;
                }
            }
        }
        else
        {
            for(u32 e = 0; e < gens.size(); ++e)
            {
                if((gens[e] & 1u) && ms_flags[e].has_all(qflags))
                {
                    ++count;
                }
            }
        }
        
        return count;
    }

    bool AddComponent(Entity e, ComponentType type)
    {
        const u32 id = e.s.id;
        if(Exists(e) && !ms_flags[id].has(type))
        {
            sc_ComponentNew[type](GetComponentData(id, type));
            ms_flags[id].set(type);
            ms_counts[type]++;
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
            ms_counts[type]--;
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
