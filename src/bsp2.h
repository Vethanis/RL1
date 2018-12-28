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

struct csgplane;
struct csgpolygon;
struct csgnode;
struct csgmodel;

// default mem
csgnode* modelToNode(const csgmodel& a);
csgmodel nodeToModel(const csgnode* a);

// temp mem
csgmodel csgunion(const csgmodel& a, const csgmodel& b);
csgmodel csgintersection(const csgmodel& a, const csgmodel& b);
csgmodel csgdifference(const csgmodel& a, const csgmodel& b);

// temp mem
csgnode* csgunion(const csgnode* a1, const csgnode* b1);
csgnode* csgdifference(const csgnode* a1, const csgnode* b1);
csgnode* csgintersect(const csgnode* a1, const csgnode* b1);

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
    Array<vec3> m_translations;
    // transform for the primitive mesh
    mat4        m_matrix;
};

struct csgmodel
{
    Array<vec3> vertices;

    csgmodel() {}
    csgmodel(const Array<vec3>& x)
    {
        vertices = x;
    }
    csgmodel(const csgnode* node)
    {
        *this = nodeToModel(node);
    }
    inline csgmodel& Translate(const vec3& x)
    {
        return Transform(glm::translate(mat4(1.0f), x));
    }
    inline csgmodel& Scale(const vec3& x)
    {
        return Transform(glm::scale(mat4(1.0f), x));
    }
    inline csgmodel& Rotate(const vec3& pyr)
    {
        return Transform(glm::eulerAngleYXZ(pyr.y, pyr.x, pyr.z));
    }
    inline csgmodel& Transform(const mat4& m)
    {
        for(vec3& v : vertices)
        {
            v = vec3(m * vec4(v, 1.0f));
        }
        return *this;
    }
    inline csgmodel Union(const csgmodel& b) const
    {
        return csgunion(*this, b);
    }
    inline csgmodel Difference(const csgmodel& b) const
    {
        return csgdifference(*this, b);
    }
    inline csgmodel Intersection(const csgmodel& b) const 
    {
        return csgintersection(*this, b);
    }
    inline csgnode* toNode()
    {
        return modelToNode(*this);
    }
};
