#include "ecs.h"

#include "gen_array.h"
#include "bitfield.h"

#include "component_funcs.h"

#if _DEBUG
    #include "imvis.h"
    #include "component_debug.h"
    #include <stdio.h>
#endif // _DEBUG

static constexpr Allocator ms_alloc =
{
    Malloc::Allocate,
    Malloc::Reallocate,
    Malloc::Free,
    4096u,
};

static GenIndices<ms_alloc>             ms_indices;                 // active slots
static Array<bool, ms_alloc>            ms_alive;                   // is entity alive
static Array<ComponentFlags, ms_alloc>  ms_flags;                   // what components exist, per-entity
static ComponentData                    ms_components[CT_Count];    // component allocations as bytes
static u32                              ms_capacity;                // capacity of component arrays

static ComponentData GetComponentData(u32 e, u32 c)
{
    return Opaque(ms_components[c], e, sc_ComponentSize[c]);
}

DebugOnly(static void DebugVis());

namespace ECS
{
    void Init()
    {

    }
    void Update()
    {
        DebugOnly(DebugVis());
    }
    void Shutdown()
    {
        const u32 cap = (u32)ms_indices.capacity();
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

        ms_flags.reset();
        ms_indices.reset();
        ms_alive.reset();
        ms_capacity = 0u;
    }

    Entity Create()
    {
        const Slot s = ms_indices.create();
        const u32 newCap = s.id + 1u;

        if (newCap > ms_capacity)
        {
            ms_capacity = newCap;
            ms_flags.resize(newCap);
            ms_alive.resize(newCap);

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

        ms_alive[s.id] = true;

        return { s };
    }

    bool Destroy(Entity e)
    {
        let id = e.s.id;
        if (ms_alive[id] && ms_indices.destroy(e.s))
        {
            ms_alive[id] = false;

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
        return ms_indices.exists(e.s) && ms_alive[e.s.id];
    }

    Slice<const ComponentFlags> GetFlags()
    {
        return Subslice(ToCSlice(ms_flags), 0, ms_capacity);
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
        let bytes = ms_indices.capacity() * sc_ComponentSize[type];
        return Subslice(ms_components[type], 0, bytes);
    }
};

#if _DEBUG
    static void DebugVis()
    {
        ImGui::Begin("ECS");

        ImGui::Text("Capacity: %u\n", ms_capacity);

        if(ImGui::Button("Add Entity"))
        {
            ECS::Create();
        }

        for(u32 e = 0; e < ms_capacity; ++e)
        {
            if(!ms_alive[e])
            {
                continue;
            }

            let ent = Entity { e, ms_indices.m_gen[e] };

            char entlabel[64];
            sprintf(entlabel, "Entity %u", e);

            if(ImGui::CollapsingHeader(entlabel))
            {
                IDScope entScope(e);
                if(ImGui::Button("Destroy Entity"))
                {
                    ECS::Destroy(ent);
                    continue;
                }

                for(u32 uType = 0; uType < CT_Count; ++uType)
                {
                    let type = (ComponentType)uType;
                    let name = ComponentName(type);

                    {
                        char button[64];
                        sprintf(button, "Add %s", name);
                        if(ImGui::Button(button))
                        {
                            ECS::AddComponent(ent, type);
                        }

                        ImGui::SameLine();

                        sprintf(button, "Remove %s", name);
                        if(ImGui::Button(button))
                        {
                            ECS::RemoveComponent(ent, type);
                        }
                    }

                    if(ms_flags[e].has(type))
                    {
                        if(ImGui::CollapsingHeader(name))
                        {
                            ImVisComponent(type, ToConst(GetComponentData(e, type)));
                        }
                    }
                }
            }
        }

        ImGui::End();
    }
#endif // _DEBUG
