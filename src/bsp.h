#pragma once

#include "linmath.h"
#include "array.h"

struct BspTree
{
    struct Node 
    {
        vec4            plane;
        Node*           back;
        Node*           front;
        Array<int32_t>  triangles;
    };

    Array<vec3> m_vertices;
    Node*       m_root;

    static constexpr int32_t SplitType(int32_t a, int32_t b, int32_t c)
    {
        return (a + 1) * 16 + (b + 1) * 4 + (c + 1);
    }
    static inline float Distance(vec4 plane, vec3 pt)
    {
        return glm::dot(vec3(plane), pt) - plane.w;
    }
    static inline float Relation(float a, float b)
    {
        return fabsf(a) / (fabsf(a) + fabsf(b));
    }
    static inline void Append(Array<int32_t>& inds, int32_t a, int32_t b, int32_t c)
    {
        inds.expand(3);
        inds.append() = a;
        inds.append() = b;
        inds.append() = c;
    }
    static inline float Sign(float x)
    {
        constexpr float e = 0.001f;
        if(x > e) return 1;
        if(x < -e) return -1;
        return 0;
    }
    static inline ivec3 Sign(vec3 x)
    {
        return ivec3(Sign(x.x), Sign(x.y), Sign(x.z));
    }
    template<typename T>
    static inline int32_t AppendInterpolate(
        T&                  x,
        const vec3&         a, 
        const vec3&         b, 
        float               alpha)
    {
        x.grow() = glm::mix(a, b, alpha);
        return x.count() - 1u;
    }
    inline const vec3& GetPosition(
        const int32_t* inds, 
        int32_t        i) const 
    {
        return m_vertices[inds[i]];
    }
    static void ResetNode(Node* n);
    inline void Reset()
    {
        ResetNode(m_root);
        m_root = nullptr;
        m_vertices.reset();
    }
    inline vec4 CalculatePlane(int32_t a, int32_t b, int32_t c) const
    {
        const vec3* verts = m_vertices.begin();
        const vec3  norm = glm::normalize(glm::cross(verts[b] - verts[a], verts[c] - verts[a]));
        const float p    = glm::dot(norm, verts[a]);
        return vec4(norm.x, norm.y, norm.z, p);
    }
    vec4 SeparateTriangles(
        int32_t                 pivot, 
        const Array<int32_t>&   indices, 
        Array<int32_t>&         back,
        Array<int32_t>&         front,
        Array<int32_t>&         onPlane);
    ivec2 EvaluatePivot(
        int32_t                pivot, 
        const Array<int32_t>&  indices);
    Node* MakeTree(const Array<int32_t>& indices);
    void SortBackToFront(
        const vec3&         p, 
        const Node*         n, 
        Array<int32_t>&    out) const;
    void SortFrontToBack(
        const vec3&         p, 
        const Node*         n, 
        Array<int32_t>&     out) const;
    bool IsInside(const vec3& p, const Node* n) const;
    void RecalculatePlanes(Node* n);
    bool ClassifyTri(
        const Node*     n,
        const vec3&     v1,
        const vec3&     v2,
        const vec3&     v3,
        bool            inside,
        bool            keepEdge,
        Array<vec3>&    out,
        bool            keepNow) const;
    void ClassifyTree(
        const Node*         tree, 
        const vec3*         verts, 
        bool                inside, 
        bool                keepEdge, 
        Array<vec3>&        out) const;
    BspTree()
    {
        m_root = nullptr;
    }
    ~BspTree()
    {
        Reset();
    }
    inline BspTree(
        Array<vec3>&           vertices, 
        const Array<int32_t>&  indices)
    {
        m_vertices.assume(vertices);
        m_root = MakeTree(indices);
    }
    BspTree(Array<vec3>& vertices)
    {
        m_vertices.assume(vertices);
        Array<int32_t> indices;
        indices.resize(vertices.count());
        for(int32_t i = 0; i < indices.count(); ++i)
        {
            indices[i] = i;
        }
        m_root = MakeTree(indices);
    }
    inline Array<int32_t> Sort(const vec3& p, bool f2b) const 
    {
        Array<int32_t> out;
        if(f2b)
        {
            SortFrontToBack(p, m_root, out);
        }
        else
        {
            SortBackToFront(p, m_root, out);
        }
        return out;
    }
    void Transform(const mat4& m);
    inline bool IsInside(const vec3& p) const 
    {
        return IsInside(p, m_root);
    }
    static inline BspTree Intersect(const BspTree& a, const BspTree& b)
    {
        Array<vec3> result;
        a.ClassifyTree(b.m_root, b.m_vertices.begin(), true, true, result);
        b.ClassifyTree(a.m_root, a.m_vertices.begin(), true, false, result);
        return BspTree(result);
    }
    static inline BspTree Add(const BspTree& a, const BspTree& b)
    {
        Array<vec3> result;
        a.ClassifyTree(b.m_root, b.m_vertices.begin(), false, true, result);
        b.ClassifyTree(a.m_root, a.m_vertices.begin(), false, false, result);
        return BspTree(result);
    }
    static inline BspTree Subtract(const BspTree& a, const BspTree& b)
    {
        Array<vec3> result;
        a.ClassifyTree(b.m_root, b.m_vertices.begin(), true, true, result);
        b.ClassifyTree(a.m_root, a.m_vertices.begin(), false, false, result);
        return result;
    }
};
