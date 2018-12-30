#pragma once

#include "slot.h"
#include "sokol_id.h"

enum ShaderType
{
    ST_Textured = 0,
    ST_Sky,
    ST_Count,
};

namespace Shaders
{
    void Create(ShaderType type, const void* desc);
    sg_shader Get(ShaderType type);
};
