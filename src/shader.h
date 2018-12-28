#pragma once

#include "slot.h"
#include "sokol_gfx.h"

enum ShaderType
{
    ST_Textured = 0,
    ST_Count,
};

namespace Shaders
{
    void Create(ShaderType type, const sg_shader_desc& desc);
    sg_shader Get(ShaderType type);
};
