#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"
#include "linmath.h"
#include "hashstring.h"

struct Buffer
{
    sg_buffer   m_vertices;
    sg_buffer   m_indices;
    int32_t     m_count;
};

struct Vertex
{
    vec3    position;
    vec3    normal;
    vec2    uv;

    inline bool operator == (const Vertex& o) const 
    {
        constexpr uint32_t len = sizeof(*this) / sizeof(uint64_t);
        const uint64_t* a = (const uint64_t*)this;
        const uint64_t* b = (const uint64_t*)&o;
        uint64_t d = 0;
        for(uint32_t i = 0; i < len; ++i)
        {
            d += a[i] - b[i];
        }
        return d == 0u;
    }
};

struct BufferData
{
    Vertex*     vertices;
    uint16_t*   indices;
    uint32_t    vertCount;
    uint32_t    indexCount;
};

namespace Buffers
{
    slot Load(BufferString name);
    void Destroy(slot s);
    void IncRef(slot s);
    void DecRef(slot s);
    const Buffer* Get(slot s);
    bool Exists(slot s);
    slot Find(const char* name);
    slot Find(Hash hash);

    BufferData Load(const char* name);
    void Free(BufferData& data);
    void Save(const char* name, const BufferData& data);
    Buffer Create(const BufferData& data);
    void Destroy(Buffer& buf);
};
