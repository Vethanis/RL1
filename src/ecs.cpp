#include "ecs.h"

#include "gen_array.h"

#include "component_types.h"
#include "component_funcs.h"

#include "imvis.h"
#include "component_debug.h"
#include <stdio.h>

static constexpr Allocator ms_alloc =
{
    Malloc::Allocate,
    Malloc::Reallocate,
    Malloc::Free,
    4096,
};

static GenIndices<ms_alloc>             ms_indices;                 // active slots
static Array<ComponentFlags, ms_alloc>  ms_flags;                   // what components exist, per-entity
static ComponentData                    ms_components[CT_Count];    // component allocations as bytes
static u32                              ms_capacity;                // capacity of component arrays

static ComponentData GetData(u32 ent, u32 comp)
{
    return Opaque(ms_components[comp], ent, sc_ComponentSize[comp]);
}

namespace ECS
{
    void Init()
    {

    }
    void Update()
    {

    }
    void Shutdown()
    {
        const u32 cap = (u32)ms_indices.capacity();
        for(u32 c = 0; c < CT_Count; ++c)
        {
            for(u32 e = 0; e < cap; ++e)
            {
                if(HasFlag(ms_flags[e], c))
                {
                    sc_ComponentDrop[c](GetData(e, c));
                }
            }
            ms_alloc.Free(ms_components[c]);
        }

        ms_flags.reset();
        ms_indices.reset();
        ms_capacity = 0;
    }

    Entity Create()
    {
        let slot = ms_indices.create();
        let e = slot.id;
        let newCap = e + 1;

        if (newCap > ms_capacity)
        {
            ms_capacity = newCap;
            ms_flags.resize(newCap);

            for(u32 i = 0; i < CT_Count; ++i)
            {
                ms_alloc.Realloc(ms_components[i], newCap * sc_ComponentSize[i]);
            }
        }

        for(u32 c = 0; c < CT_Count; ++c)
        {
            EraseComp(GetData(e, c));
        }
        EraseR(ms_flags[e]);

        return { slot };
    }

    bool Destroy(Entity e)
    {
        let id = e.s.id;
        if (ms_indices.destroy(e.s))
        {
            for(u32 c = 0; c < CT_Count; ++c)
            {
                if(HasFlag(ms_flags[id], c))
                {
                    sc_ComponentDrop[c](GetData(id, c));
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
        return ToCSlice(ms_flags);
    }
    CDwordSlice GetGens()
    {
        return ToConst(ms_indices.gens());
    }

    bool AddComponent(Entity e, ComponentType type)
    {
        let id = e.s.id;
        let uType = (u32)type;

        if(Exists(e) && !HasFlag(ms_flags[id], uType))
        {
            sc_ComponentNew[uType](GetData(id, uType));
            SetFlag(ms_flags[id], uType);
            return true;
        }
        return false;
    }

    bool RemoveComponent(Entity e, ComponentType type)
    {
        let id = e.s.id;
        let uType = (u32)type;

        if(Exists(e) && HasFlag(ms_flags[id], uType))
        {
            sc_ComponentDrop[uType](GetData(id, uType));
            UnsetFlag(ms_flags[id], uType);
            return true;
        }
        return false;
    }

    ComponentData GetComponent(Entity e, ComponentType type)
    {
        let id = e.s.id;
        let uType = (u32)type;

        if(Exists(e) && HasFlag(ms_flags[id], uType))
        {
            return GetData(id, uType);
        }
        return { 0, 0 };
    }

    ComponentData GetAllComponents(ComponentType type)
    {
        let bytes = ms_indices.capacity() * sc_ComponentSize[type];
        return Subslice(ms_components[type], 0, bytes);
    }

    static bool ms_imvisEnabled = false;
    bool& ImVisEnabled()
    {
        return ms_imvisEnabled;
    }
    void ImVisUpdate()
    {
        ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("ECS");

        u32 count = 0;
        {
            let gens = ECS::GetGens();
            for(let gen : gens)
            {
                count += gen & 1;
            }
            ImGui::Text("Count:    %u\n", count);
            ImGui::Text("Capacity: %zu\n", gens.size());
        }
        ImGui::Separator();
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
            let flags   = ECS::GetFlags();
            let gens    = ECS::GetGens();
            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(gens[e] & 1)
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
                ImGui::Text("ID, Gen");
                ImGui::NextColumn();
            }
            let flags = ECS::GetFlags();
            let gens = ECS::GetGens();
            for(u32 e = 0; e < flags.size(); ++e)
            {
                if(gens[e] & 1)
                {
                    ImGui::Text("%u, %u", e, gens[e] >> 1);
                    ImGui::NextColumn();
                }
            }
        }
        ImGui::Separator();
        {
            // component rows
            let flags = ECS::GetFlags();
            let gens = ECS::GetGens();
            for(u32 c = 0; c < CT_Count; ++c)
            {
                let type = (ComponentType)c;

                ImGui::Columns(count + 1);

                ImGui::Text("%s", ComponentName(type));
                ImGui::NextColumn();

                for(u32 e = 0; e < flags.size(); ++e)
                {
                    if(!(gens[e] & 1))
                    {
                        continue;
                    }

                    IDScope addbtn("ECS::AddComponent", c, e);
                    if(!HasFlag(flags[e], c) && ImGui::Button("+"))
                    {
                        ECS::AddComponent({ e, gens[e] }, type);
                    }

                    IDScope rembtn("ECS::RemoveComponent", c, e);
                    if(HasFlag(flags[e], c) && ImGui::Button("-"))
                    {
                        ECS::RemoveComponent({ e, gens[e] }, type);
                    }

                    if(HasFlag(flags[e], c))
                    {
                        ImGui::SameLine();
                        ImVisComponent(type, ToConst(GetData(e, c)));
                    }

                    ImGui::NextColumn();
                }

                ImGui::Separator();
            }
        }

        ImGui::End();
    }
};
