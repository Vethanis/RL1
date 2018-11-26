#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"
#include "linmath.h"

struct Buffer
{
    sg_buffer   m_id;
    int32_t     m_count;
};

struct Vertex
{
    vec3    position;
    vec3    normal;
    vec2    uv;
    float   ao;
};

namespace Buffers
{
    slot Load(const char* name);
    slot Create(
        const char*     name,
        const Vertex*   vertices, 
        uint32_t        vertCount);
    void Destroy(slot s);
    const Buffer* Get(slot s);
    bool Exists(slot s);
    bool Exists(const char* name);
    slot Find(const char* name);
    slot Find(uint64_t hash);
    void Save(
        const char*     name, 
        const Vertex*   vertices, 
        uint32_t        count);
};
