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
#include "swap.h"
#include "allocator.h"

struct csgplane;
struct csgpolygon;
struct csgnode;

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

// temp mem
csgmodel csgunion(const csgmodel& a, const csgmodel& b);
csgmodel csgintersection(const csgmodel& a, const csgmodel& b);
csgmodel csgdifference(const csgmodel& a, const csgmodel& b);

// temp mem
csgnode* csgunion(const csgnode* a1, const csgnode* b1);
csgnode* csgdifference(const csgnode* a1, const csgnode* b1);
csgnode* csgintersect(const csgnode* a1, const csgnode* b1);

typedef Array<csgpolygon, false> polylist;

struct csgplane
{
    vec3 normal;
    float w;

    csgplane() {}
    csgplane(const vec3& a, const vec3& b, const vec3& c)
    {
        normal = glm::normalize(glm::cross(b - a, c - a));
        w = glm::dot(normal, a);
    }
    inline bool ok() const 
    {
        return glm::length(normal) > 0.0f;
    }
    inline void flip()
    {
        normal *= -1.0f;
        w *= -1.0f;
    }
    inline float distance(const vec3& x) const 
    {
        return glm::dot(normal, x) - w;
    }
    void splitPolygon(
        const csgpolygon& polygon, 
        polylist& cofront,
        polylist& coback,
        polylist& front,
        polylist& back) const;
};

struct csgpolygon
{
    Array<vec3> vertices;
    csgplane plane;

    csgpolygon() {}
    csgpolygon(const Array<vec3>& list)
    {
        plane = csgplane(list[0], list[1], list[2]);
        vertices = list;
    }
    inline void flip()
    {
        plane.flip();
        for(int32_t i = 0; i < vertices.count() / 2; ++i)
        {
            Swap(vertices[i], vertices[vertices.count() - i - 1]);
        }
    }
};

struct csgnode
{
    polylist polygons;
    csgnode* front;
    csgnode* back;
    csgplane plane;

    csgnode()
    {
        memset(this, 0, sizeof(*this));
    }
    csgnode(const polylist& list)
    {
        memset(this, 0, sizeof(*this));
        build(list);
    }
    ~csgnode()
    {
        Allocator::Delete(front);
        Allocator::Delete(back);
    }
    csgnode* clone() const;
    void clipTo(const csgnode* other);
    void invert();
    void build(const polylist& list);
    polylist clipPolygons(const polylist& list) const;
    polylist allPolygons() const;
};

enum csgop
{
    Union = 0,
    Intersection,
    Difference,
    CSGOP_COUNT,
};

struct csg
{
    csgnode* m_node;
    int32_t  m_type;
};

// default mem
csgnode* modelToNode(const csgmodel& a);
csgmodel nodeToModel(const csgnode* a);
