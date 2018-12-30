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
    const TempArray<vec3>&      verts, 
    TempArray<Vertex>&          out, 
    TempArray<int32_t>&         indout);

void PositionsToVertices(
    const TempArray<vec3>&      verts, 
    const TempArray<int32_t>&   inds, 
    TempArray<Vertex>&          out);
