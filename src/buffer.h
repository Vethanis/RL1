#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"
#include "linmath.h"
#include "hashstring.h"
#include "vertex.h"

struct Buffer
{
    sg_buffer   m_vertices;
    sg_buffer   m_indices;
    int32_t     m_count;
};

struct BufferData
{
    Vertex*     vertices;
    uint32_t*   indices;
    uint32_t    vertCount;
    uint32_t    indexCount;
};

namespace Buffers
{
    slot Load(BufferString name);
    slot Create(BufferString name, const Buffer& buf);
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
