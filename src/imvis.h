#pragma once

#include "imgui.h"
#include "macro.h"
#include "fnv.h"

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
