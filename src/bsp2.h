#pragma once

// Original CSG.JS library by Evan Wallace (http://madebyevan.com), under the MIT license.
// GitHub: https://github.com/evanw/csg.js/
// 
// C++ port by Tomasz Dabrowski (http://28byteslater.com), under the MIT license.
// GitHub: https://github.com/dabroz/csgjs-cpp/
// 
// Constructive Solid Geometry (CSG) is a modeling technique that uses Boolean
// operations like union and intersection to combine 3D solids. This library
// implements CSG operations on meshes elegantly and concisely using BSP trees,
// and is meant to serve as an easily understandable implementation of the
// algorithm. All edge cases involving overlapping coplanar polygons in both
// solids are correctly handled.
//
// To use this as a header file, define CSGJS_HEADER_ONLY before including this file.
//

// Edited again by Kara Hughes, under MIT license.

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

typedef Array<csg, false> csglist;

void SetCSGPrim(csgprim type, const Array<vec3>& vertices);
void Evaluate(const csglist& list, Array<vec3>& out);
