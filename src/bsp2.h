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

struct csgmodel
{
    Array<vec3> vertices;

    csgmodel() {}
    csgmodel(const Array<vec3>& x)
    {
        vertices = x;
    }
    csgmodel(const Array<vec3>& x, const vec3& translation)
    {
        vertices = x;
        Translate(translation);
    }
    inline void Translate(const vec3& x)
    {
        mat4 m = glm::translate(mat4(1.0f), x);
        for(vec3& v : vertices)
        {
            v = vec3(m * vec4(v, 1.0f));
        }
    }
};

csgmodel csgunion(const csgmodel& a, const csgmodel& b);
csgmodel csgintersection(const csgmodel& a, const csgmodel& b);
csgmodel csgdifference(const csgmodel& a, const csgmodel& b);
