#pragma once

#include "slot.h"
#include "sokol_gfx.h"

namespace Shaders
{
    slot Create(const char* name, const sg_shader_desc& desc);
    void Destroy(slot s);
    sg_shader Get(slot s);
    bool Exists(slot s);
    slot Find(const char* name);
    slot Find(uint64_t hash);
};
