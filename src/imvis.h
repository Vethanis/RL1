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
    IDScope(CByteSlice id)
    {
        ImGui::PushID((i32)Fnv32(id));
    }
    ~IDScope()
    {
        ImGui::PopID();
    }
};
