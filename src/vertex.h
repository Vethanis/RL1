#pragma once

#include <stdint.h>
#include "linmath.h"
#include "array.h"
#include "dict.h"

struct Vertex
{
    vec3    position;
    vec3    normal;
};

void IndexVertices(
    const TempArray<Vertex>&    verts, 
    TempArray<Vertex>&          out, 
    TempArray<int32_t>&         indout);

void PositionsToVertices(
    const TempArray<vec3>&      verts, 
    const TempArray<int32_t>&   inds, 
    TempArray<Vertex>&          out);
