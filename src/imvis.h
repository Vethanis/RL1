#pragma once

#include "imgui.h"
#include "lang.h"
#include "fnv.h"
#include "hlsl_types.h"

struct IDScope
{
    IDScope(i32 id)
    {
        ImGui::PushID(id);
    }
    IDScope(cstr id)
    {
        ImGui::PushID((i32)Fnv32(id));
    }
    IDScope(cstr name, u32 idx)
    {
        let id =
            Fnv32(name) ^
            (Fnv32(&idx, sizeof(idx)) << 16u);
        ImGui::PushID((i32)id);
    }
    IDScope(cstr name, u32 row, u32 col)
    {
        let id =
            Fnv32(name) ^
            (Fnv32(&row, sizeof(row)) << 10u) ^
            (Fnv32(&col, sizeof(col)) << 20u);
        ImGui::PushID((i32)id);
    }
    IDScope(const void* ptr, usize len)
    {
        ImGui::PushID((i32)Fnv32(ptr, len));
    }
    ~IDScope()
    {
        ImGui::PopID();
    }
};

inline void ImVisCstr(cstr x)
{
    ImGui::Text("%s", x);
}

inline void ImVisF32(f32 x)
{
    ImGui::Text("%g", x);
}

inline void ImVisFloat2(float2 x)
{
    ImGui::Text("%g %g", x.x, x.y);
}

inline void ImVisFloat3(float3 x)
{
    ImGui::Text("%g %g %g", x.x, x.y, x.z);
}

inline void ImVisFloat4(float4 x)
{
    ImGui::Text("%g %g %g %g", x.x, x.y, x.z, x.w);
}
