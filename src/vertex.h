#pragma once

#include <stdint.h>
#include "linmath.h"
#include "array.h"

struct Vertex
{
    vec3    position;
    vec3    normal;

    inline bool operator==(const Vertex& other) const 
    {
        const uint64_t* a = (const uint64_t*)this;
        const uint64_t* b = (const uint64_t*)&other;
        uint64_t d = 0;
        for(uint32_t i = 0; i < 3; ++i)
        {
            d += a[i] - b[i];
        }
        return d == 0;
    }
};

void PositionsToVertices(
    const Array<vec3>&      verts, 
    Array<Vertex>&          out, 
    Array<int32_t>&         indout);

void PositionsToVertices(
    const Array<vec3>&      verts, 
    const Array<int32_t>&   inds, 
    Array<Vertex>&          out);
