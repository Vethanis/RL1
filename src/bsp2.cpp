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

#include "bsp2.h"
#include "swap.h"
#include "allocator.h"

struct csgplane;
struct csgpolygon;
struct csgnode;
struct csgmodel;

const Array<vec3>& GetCSGPrim(csgprim type);

csgnode* modelToNode(const csgmodel& a);
csgmodel nodeToModel(const csgnode* a);

csgmodel csgunion(const csgmodel& a, const csgmodel& b);
csgmodel csgintersection(const csgmodel& a, const csgmodel& b);
csgmodel csgdifference(const csgmodel& a, const csgmodel& b);

csgnode* csgunion(const csgnode* a1, const csgnode* b1);
csgnode* csgdifference(const csgnode* a1, const csgnode* b1);
csgnode* csgintersect(const csgnode* a1, const csgnode* b1);

struct csgmodel
{
    TempArray<vec3> vertices;

    csgmodel() {}
    csgmodel(const TempArray<vec3>& x)
    {
        vertices = x;
    }
    csgmodel(const Array<vec3>& x)
    {
        vertices.resize(x.count());
        memcpy(vertices.begin(), x.begin(), x.bytes());
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

typedef TempArray<csgpolygon, false> polylist;

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
    TempArray<vec3> vertices;
    csgplane plane;

    csgpolygon() {}
    csgpolygon(const TempArray<vec3>& list)
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
        Allocator::Delete(AB_Temp, front);
        Allocator::Delete(AB_Temp, back);
    }
    csgnode* clone() const;
    void clipTo(const csgnode* other);
    void invert();
    void build(const polylist& list);
    polylist clipPolygons(const polylist& list) const;
    polylist allPolygons() const;
};

void csgplane::splitPolygon(
    const csgpolygon& polygon, 
    polylist& cofront,
    polylist& coback,
    polylist& front,
    polylist& back) const
{
    enum
    {
        Coplanar = 0,
        Front,
        Back,
        Spanning,
    };

    uint32_t polygonType = 0u;
    TempArray<uint32_t> types;

    types.expand(polygon.vertices.count());
    for(const auto& x : polygon.vertices)
    {
        const float e = 0.001f;
        float t = distance(x);
        uint32_t type = (t < -e) ? Back : ((t > e) ? Front : Coplanar);
        polygonType |= type;
        types.append() = type;
    }

    switch(polygonType)
    {
        case Coplanar:
        {
            if(glm::dot(normal, polygon.plane.normal) > 0.0f)
            {
                cofront.grow() = polygon;
            }
            else
            {
                coback.grow() = polygon;
            }
        }
        break;
        case Front:
        {
            front.grow() = polygon;
        }
        break;
        case Back:
        {
            back.grow() = polygon;
        }
        break;
        case Spanning:
        {
            TempArray<vec3> f, b;
            for(int32_t i = 0; i < polygon.vertices.count(); ++i)
            {
                int32_t j = (i + 1) % polygon.vertices.count();
                int32_t ti = types[i];
                int32_t tj = types[j];
                vec3 vi = polygon.vertices[i];
                vec3 vj = polygon.vertices[j];
                if(ti != Back)
                {
                    f.grow() = vi;
                }
                if(ti != Front)
                {
                    b.grow() = vi;
                }
                if((ti | tj) == Spanning)
                {
                    float t = (w - glm::dot(normal, vi)) / glm::dot(normal, vj - vi);
                    vec3 v = glm::mix(vi, vj, t);
                    f.grow() = v;
                    b.grow() = v;
                }
            }
            if(f.count() >= 3)
            {
                front.grow() = csgpolygon(f);
            }
            if(b.count() >= 3)
            {
                back.grow() = csgpolygon(b);
            }
        }
        break;
    }
}

csgnode* csgunion(const csgnode* a1, const csgnode* b1)
{
    csgnode* a = a1->clone();
    csgnode* b = b1->clone();
    a->clipTo(b);
    b->clipTo(a);
    b->invert();
    b->clipTo(a);
    b->invert();
    a->build(b->allPolygons());
    csgnode* ret = Allocator::New<csgnode>(AB_Temp);
    new (ret) csgnode(a->allPolygons());
    return ret;
}

csgnode* csgdifference(const csgnode* a1, const csgnode* b1)
{
	csgnode* a = a1->clone();
	csgnode* b = b1->clone();
	a->invert();
	a->clipTo(b);
	b->clipTo(a);
	b->invert();
	b->clipTo(a);
	b->invert();
	a->build(b->allPolygons());
	a->invert();
    csgnode* ret = Allocator::New<csgnode>(AB_Temp);
    new (ret) csgnode(a->allPolygons());
    return ret;
}

csgnode* csgintersect(const csgnode* a1, const csgnode* b1)
{
	csgnode* a = a1->clone();
	csgnode* b = b1->clone();
	a->invert();
	b->clipTo(a);
	b->invert();
	a->clipTo(b);
	b->clipTo(a);
	a->build(b->allPolygons());
	a->invert();
    csgnode* ret = Allocator::New<csgnode>(AB_Temp);
    new (ret) csgnode(a->allPolygons());
	return ret;
}

void csgnode::invert()
{
    for(csgpolygon& poly : polygons)
    {
        poly.flip();
    }
    plane.flip();
    Swap(front, back);
    if(front)
    {
        front->invert();
    }
    if(back)
    {
        back->invert();
    }
}

struct ClipPair
{
    const csgnode* node;
    polylist list;
};

static void csgnode_clipPolygons(const ClipPair& pair, polylist& result)
{
    if(!pair.node->plane.ok())
    {
        result.expand(pair.list.count());
        for(const auto& x : pair.list)
        {
            result.append() = x;
        }
        return;
    }

    polylist lfront, lback;
    for(int32_t i = 0; i < pair.list.count(); ++i)
    {
        pair.node->plane.splitPolygon(pair.list[i], lfront, lback, lfront, lback);
    }

    if(pair.node->front)
    {
        csgnode_clipPolygons({ pair.node->front, lfront }, result);
    }
    else
    {
        result.expand(lfront.count());
        for(const auto& x : lfront)
        {
            result.append() = x;
        }
    }

    if(pair.node->back)
    {
        csgnode_clipPolygons({ pair.node->back, lback }, result);
    }
}

polylist csgnode::clipPolygons(const polylist& list) const 
{
    polylist result;

    csgnode_clipPolygons({ this, list }, result);

    return result;
}

void csgnode::clipTo(const csgnode* other)
{
    polygons = other->clipPolygons(polygons);
    if(front)
    {
        front->clipTo(other);
    }
    if(back)
    {
        back->clipTo(other);
    }
}

static void csgnode_allPolygons(polylist& result, const csgnode* x)
{
    if(x)
    {
        result.expand(x->polygons.count());
        for(const auto& p : x->polygons)
        {
            result.append() = p;
        }
        csgnode_allPolygons(result, x->front);
        csgnode_allPolygons(result, x->back);
    }
}

polylist csgnode::allPolygons() const 
{
    polylist result;
    csgnode_allPolygons(result, this);
    return result;
}

csgnode* csgnode::clone() const 
{
    csgnode* node = Allocator::New<csgnode>(AB_Temp);
    node->polygons = polygons;
    node->plane = plane;
    if(front)
    {
        node->front = front->clone();
    }
    if(back)
    {
        node->back = back->clone();
    }
    return node;
}

void csgnode::build(const polylist& list)
{
    if(list.empty())
    {
        return;
    }

    if(!plane.ok())
    {
        plane = list[0].plane;
    }
    polylist lfront, lback;
    for(const auto& x : list)
    {
        plane.splitPolygon(x, polygons, polygons, lfront, lback);
    }
    if(lfront.count())
    {
        if(!front)
        {
            front = Allocator::New<csgnode>(AB_Temp);
        }
        front->build(lfront);
    }
    if(lback.count())
    {
        if(!back)
        {
            back = Allocator::New<csgnode>(AB_Temp);
        }
        back->build(lback);
    }
}

polylist modelToPolygon(const csgmodel& model)
{
    polylist list;
    list.expand(model.vertices.count() / 3);
    for(int32_t i = 0; i + 2 < model.vertices.count(); i += 3)
    {
        TempArray<vec3> tri;
        tri.resize(3);
        for(int32_t j = 0; j < 3; ++j)
        {
            vec3 v = model.vertices[i + j];
            tri[j] = v;
        }
        list.append() = csgpolygon(tri);
    }
    return list;
}

csgmodel modelFromPolygon(const polylist& polygons)
{
    csgmodel model;
    model.vertices.expand(polygons.count() * 3);
    for(const csgpolygon& poly : polygons)
    {
        for(int32_t j = 2; j < poly.vertices.count(); j++)
        {
            model.vertices.grow() = poly.vertices[0];
            model.vertices.grow() = poly.vertices[j - 1];
            model.vertices.grow() = poly.vertices[j];
        }
    }
    return model;
}

typedef csgnode* (*csgfunc)(const csgnode*, const csgnode*);

static inline csgmodel csgop(const csgmodel& a, const csgmodel& b, csgfunc fn)
{
    csgnode* A = Allocator::Alloc<csgnode>(AB_Temp);
    csgnode* B = Allocator::Alloc<csgnode>(AB_Temp);
    new (A) csgnode(modelToPolygon(a));
    new (B) csgnode(modelToPolygon(b));
    csgnode* AB = fn(A, B);
    polylist polygons = AB->allPolygons();
    return modelFromPolygon(polygons);
}

csgmodel csgunion(const csgmodel& a, const csgmodel& b)
{
    return csgop(a, b, csgunion);
}

csgmodel csgintersection(const csgmodel& a, const csgmodel& b)
{
    return csgop(a, b, csgintersect);
}

csgmodel csgdifference(const csgmodel& a, const csgmodel& b)
{
    return csgop(a, b, csgdifference);
}

csgnode* modelToNode(const csgmodel& a)
{
    csgnode* A = Allocator::Alloc<csgnode>(AB_Temp);
    new (A) csgnode(modelToPolygon(a));
    return A;
}

csgmodel nodeToModel(const csgnode* a)
{
    return modelFromPolygon(a->allPolygons());
}

Array<vec3> ms_prims[CSGPRIM_COUNT];

void SetCSGPrim(csgprim type, const TempArray<vec3>& vertices)
{
    ms_prims[type].resize(vertices.count());
    memcpy(ms_prims[type].begin(), vertices.begin(), vertices.bytes());
}

const Array<vec3>& GetCSGPrim(csgprim type)
{
    return ms_prims[type];
}

void Evaluate(const csglist& list, TempArray<vec3>& out)
{
    out.clear();
    if(list.empty())
    {
        return;
    }
    csgmodel A = csgmodel(GetCSGPrim(list[0].m_prim));
    A.Transform(list[0].m_matrix);
    for(int32_t i = 1; i < list.count(); ++i)
    {
        csgmodel B = csgmodel(GetCSGPrim(list[i].m_prim));
        B.Transform(list[i].m_matrix);
        switch(list[i].m_op)
        {
            case Union:
            {
                A = csgunion(A, B);
            }
            break;
            case Intersection:
            {
                A = csgintersection(A, B);
            }
            break;
            case Difference:
            {
                A = csgdifference(A, B);
            }
            break;
        }
    }
    out.resize(A.vertices.count());
    memcpy(out.begin(), A.vertices.begin(), out.bytes());
}
