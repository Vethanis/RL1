#pragma once

#include <stdint.h>
#include "linmath.h"
#include "array.h"

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

void PositionsToVertices(
    const Array<vec3>&      verts, 
    const Array<int32_t>&   inds, 
    Array<Vertex>&          out);
