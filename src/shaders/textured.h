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
    vec3        Pal0;
    vec3        Pal1;
    vec3        Pal2;
    float       PalCenter;
    float       RoughnessOffset;
    float       MetalnessOffset;
    float       Seed;
};

extern const char* textured_fs;
