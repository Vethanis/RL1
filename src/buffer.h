#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"

struct Buffer
{
    sg_buffer   m_id;
    int32_t     m_count;
};

struct Vertex
{
    float position[3];
    float uv[2];
};

namespace Buffers
{
    slot Create(const char* name);
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
    static void Save(
        const char*     name, 
        const Vertex*   vertices, 
        uint32_t        count);
};
