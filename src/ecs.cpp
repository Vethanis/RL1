#include "ecs.h"

#include "gen_array.h"
#include "bitfield.h"

#include "component_types.h"
#include "component_funcs.h"

#if _DEBUG || 1
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

static void DebugVis();

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

#if _DEBUG || 1

    static void DebugVis()
    {
        let flags = ECS::GetFlags();
        let gens = ms_indices.gens();

        ImGui::Begin("ECS");

        ImGui::Text("Entity Capacity: %u\n", ms_capacity);

        u32 count = 0;
        for(let alive : ms_alive)
        {
            count += alive ? 1 : 0;
        }

        {
            // add / remove entity row
            ImGui::Columns(count + 1);
            {
                IDScope btn("ECS::Create");
                if(ImGui::Button("+"))
                {
                    ECS::Create();
                }
                ImGui::NextColumn();
            }
            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(ms_alive[e])
                {
                    IDScope btn("ECS::Destroy", e);
                    if(ImGui::Button("-"))
                    {
                        ECS::Destroy({ e, gens[e] });
                    }
                    ImGui::NextColumn();
                }
            }
        }

        ImGui::Separator();

        {
            // ID row
            ImGui::Columns(count + 1);
            {
                ImGui::Text("ID");
                ImGui::NextColumn();
            }
            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(ms_alive[e])
                {
                    ImGui::Text("%u", e);
                    ImGui::NextColumn();
                }
            }
        }

        ImGui::Separator();

        {
            // gen row
            ImGui::Columns(count + 1);
            {
                ImGui::Text("Gen");
                ImGui::NextColumn();
            }
            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(ms_alive[e])
                {
                    ImGui::Text("%u", gens[e]);
                    ImGui::NextColumn();
                }
            }
        }

        ImGui::Separator();

        // component rows
        for(u32 c = 0; c < CT_Count; ++c)
        {
            let type = (ComponentType)c;

            ImGui::Columns(count + 1);

            ImGui::Text("%s", ComponentName(type));
            ImGui::NextColumn();

            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(!ms_alive[e])
                {
                    continue;
                }

                IDScope addbtn("ECS::AddComponent", c, e);
                if(!flags[e].has(type) && ImGui::Button("+"))
                {
                    ECS::AddComponent({ e, gens[e] }, type);
                }

                IDScope rembtn("ECS::RemoveComponent", c, e);
                if(flags[e].has(type) && ImGui::Button("-"))
                {
                    ECS::RemoveComponent({ e, gens[e] }, type);
                }

                if(flags[e].has(type))
                {
                    ImGui::SameLine();
                    let data = ToConst(GetComponentData(e, c));
                    ImVisComponent(type, data);
                }

                ImGui::NextColumn();
            }

            ImGui::Separator();
        }

        ImGui::End();
    }
#endif // _DEBUG
