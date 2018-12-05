#pragma once

#include "../linmath.h"

struct VSUniform
{
    mat4 MVP;
    mat4 M;
};

extern const char* textured_vs;

struct FSUniform
{
    vec3        Eye;
    vec3        LightDir;
    vec3        LightRad;
    float       BumpScale;
    float       RoughnessOffset;
    float       MetalnessOffset;
    float       Seed;
};

extern const char* textured_fs;
