#pragma once

#include <stdint.h>
#include "slot.h"
#include "renderer.h"

struct Buffer
{
    Renderer::Buffer   m_vertices;
    Renderer::Buffer   m_indices;
    uint32_t           m_count;
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
