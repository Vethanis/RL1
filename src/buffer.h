#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"
#include "linmath.h"
#include "vertex.h"

struct Buffer
{
    sg_buffer   m_vertices;
    sg_buffer   m_indices;
    int32_t     m_count;
};

struct BufferData
{
    void*       vertices;
    uint32_t*   indices;
    uint32_t    vertCount;
    uint32_t    indexCount;
    uint32_t    size;   // size of each vertex
};

namespace Buffers
{
    slot Create(const BufferData& data);
    void Destroy(slot s);
    const Buffer* Get(slot s);
};
