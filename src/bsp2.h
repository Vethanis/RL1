#pragma once

#include "linmath.h"
#include "array.h"

enum csgop
{
    Union = 0,
    Intersection,
    Difference,
    CSGOP_COUNT,
};

enum csgprim
{
    Box = 0,
    Sphere,
    CSGPRIM_COUNT,
};

struct csg
{
    csgop       m_op;
    csgprim     m_prim;
    // a translation for each control point on a csg mesh
    // Array<vec3> m_translations;
    // transform for the primitive mesh
    mat4        m_matrix;
};

typedef TempArray<csg, false> csglist;

void SetCSGPrim(csgprim type, const TempArray<vec3>& vertices);
void Evaluate(const csglist& list, TempArray<vec3>& out);
