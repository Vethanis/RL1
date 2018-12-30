#pragma once

#include "../linmath.h"

namespace Sky
{
    struct VSUniform
    {

    };

    struct FSUniform
    {
        mat4    IVP;
        vec3    Eye;
        vec3    LightDir;
        float   LightRad;
    };

    const void* GetShaderDesc();
    const void* GetPipelineDesc();
};
