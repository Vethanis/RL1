#pragma once

#include "../linmath.h"

namespace Flat
{
    struct VSUniform
    {
        mat4 MVP;
        mat4 M;
    };

    struct FSUniform
    {
        vec3        Eye;
        vec3        LightDir;
        vec3        Albedo;
        float       LightRad;
        float       Roughness;
        float       Metalness;
        float       Seed;
    };
};

extern const char* flat_vs;
extern const char* flat_fs;