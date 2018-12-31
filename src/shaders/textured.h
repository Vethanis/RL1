#pragma once

#include "../linmath.h"

namespace Textured
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
        vec3        Pal0;
        vec3        Pal1;
        vec3        Pal2;
        float       PalCenter;
        float       LightRad;
        float       RoughnessOffset;
        float       MetalnessOffset;
        float       Seed;
    };
};

extern const char* textured_vs;
extern const char* textured_fs;
