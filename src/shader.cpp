#include "shader.h"

#include "macro.h"

namespace Shaders
{
    sg_shader shaders[ST_Count];

    void Create(ShaderType type, const sg_shader_desc& desc)
    {
        shaders[type] = sg_make_shader(&desc);
        Assert(shaders[type].id != SG_INVALID_ID);
    }
    sg_shader Get(ShaderType type)
    {
        return shaders[type];
    }
};
