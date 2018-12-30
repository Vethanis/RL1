#include "shader.h"

#include "sokol_gfx.h"
#include "macro.h"

namespace Shaders
{
    sg_shader shaders[ST_Count];

    void Create(ShaderType type, const void* desc)
    {
        shaders[type] = sg_make_shader((const sg_shader_desc*)desc);
        Assert(shaders[type].id != SG_INVALID_ID);
    }
    sg_shader Get(ShaderType type)
    {
        return shaders[type];
    }
};
