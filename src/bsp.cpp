#include "bsp.h"

#include "prng.h"

void BspTree::ResetNode(Node* n)
{
    if(n)
    {
        n->triangles.reset();
        ResetNode(n->front);
        ResetNode(n->back);
        m_nodes.Free(n);
    }
}

vec4 BspTree::SeparateTriangles(
    int32_t                 pivot, 
    const Array<int32_t>&   indices, 
    Array<int32_t>&         back,
    Array<int32_t>&         front,
    Array<int32_t>&         onPlane)
{
    back.clear();
    front.clear();
    onPlane.clear();
    
    vec4 plane = CalculatePlane(
        indices[pivot], 
        indices[pivot + 1], 
        indices[pivot + 2]);

    const vec3*    vecs     = m_vertices.begin();
    const int32_t* inds     = indices.begin();
    const int32_t  indcount = indices.count();
    for(int32_t i = 0; i + 2 < indcount; i += 3)
    {
        vec3 dist = vec3(
            Distance(plane, vecs[inds[i + 0]]),
            Distance(plane, vecs[inds[i + 1]]),
            Distance(plane, vecs[inds[i + 2]])
        );

        ivec3 side = Sign(dist);
        uvec3 A;

        if(side[0] * side[1] == -1)
        {
            A[0] = AppendInterpolate(
                m_vertices,
                GetPosition(inds, i + 0),
                GetPosition(inds, i + 1),
                Relation(dist[0], dist[1]));
        }
        if(side[1] * side[2] == -1)
        {
            A[1] = AppendInterpolate(
                m_vertices,
                GetPosition(inds, i + 1),
                GetPosition(inds, i + 2),
                Relation(dist[1], dist[2]));
        }
        if(side[2] * side[0] == -1)
        {
            A[2] = AppendInterpolate(
                m_vertices,
                GetPosition(inds, i + 2),
                GetPosition(inds, i + 0),
                Relation(dist[2], dist[0]));
        }

        switch(SplitType(side[0], side[1], side[2]))
        {
            case SplitType(-1, -1, -1):
            case SplitType(-1, -1,  0):
            case SplitType(-1,  0, -1):
            case SplitType(-1,  0,  0):
            case SplitType( 0, -1, -1):
            case SplitType( 0, -1,  0):
            case SplitType( 0,  0, -1):
            Append(back,    inds[i], inds[i+1], inds[i+2]);
            break;

            case SplitType( 0,  0,  1):
            case SplitType( 0,  1,  0):
            case SplitType( 0,  1,  1):
            case SplitType( 1,  0,  0):
            case SplitType( 1,  0,  1):
            case SplitType( 1,  1,  0):
            case SplitType( 1,  1,  1):
            Append(front,   inds[i  ], inds[i+1], inds[i+2]);
            break;

            case SplitType( 0,  0,  0):
            Append(onPlane, inds[i  ], inds[i+1], inds[i+2]);
            break;

            case SplitType( 1, -1,  0):
            Append(back,    inds[i+1], inds[i+2],   A[0]);
            Append(front,   inds[i+2], inds[i+0],   A[0]);
            break;

            case SplitType(-1,  0,  1):
            Append(back,    inds[i+0], inds[i+1],   A[2]);
            Append(front,   inds[i+1], inds[i+2],   A[2]);
            break;

            case SplitType( 0,  1, -1):
            Append(back,    inds[i+2], inds[i+0],   A[1]);
            Append(front,   inds[i+0], inds[i+1],   A[1]);
            break;

            case SplitType(-1,  1,  0):
            Append(back,    inds[i+2], inds[i+0],   A[0]);
            Append(front,   inds[i+1], inds[i+2],   A[0]);
            break;

            case SplitType( 1,  0, -1):
            Append(back,    inds[i+1], inds[i+2],   A[2]);
            Append(front,   inds[i+0], inds[i+1],     A[2]);
            break;

            case SplitType( 0, -1,  1):
            Append(back,    inds[i+0], inds[i+1],   A[1]);
            Append(front,   inds[i+2], inds[i+0],   A[1]);
            break;

            case SplitType( 1, -1, -1):
            Append(front,   inds[i+0], A[0],        A[2]);
            Append(back,    inds[i+1], A[2],        A[0]);
            Append(back,    inds[i+1], inds[i+2],   A[2]);
            break;

            case SplitType(-1,  1, -1):
            Append(front,   inds[i+1], A[1],        A[0]);
            Append(back,    inds[i+2], A[0],        A[1]);
            Append(back,    inds[i+2], inds[i+0],   A[0]);
            break;

            case SplitType(-1, -1,  1):
            Append(front,   inds[i+2], A[2],        A[1]);
            Append(back,    inds[i+0], A[1],        A[2]);
            Append(back,    inds[i+0], inds[i+1],   A[1]);
            break;

            case SplitType(-1,  1,  1):
            Append(back,    inds[i+0], A[0],        A[2]);
            Append(front,   inds[i+1], A[2],        A[0]);
            Append(front,   inds[i+1], inds[i+2],   A[2]);
            break;

            case SplitType( 1, -1,  1):
            Append(back,    inds[i+1], A[1],        A[0]);
            Append(front,   inds[i+0], A[0],        A[1]);
            Append(front,   inds[i+2], inds[i+0],   A[1]);
            break;

            case SplitType( 1,  1, -1):
            Append(back,    inds[i+2], A[2],         A[1]);
            Append(front,   inds[i+0], A[1],         A[2]);
            Append(front,   inds[i+0], inds[i+1],    A[1]);
            break;
        }
    }
    return plane;
}

ivec2 BspTree::EvaluatePivot(
    int32_t                pivot, 
    const Array<int32_t>&  indices)
{
    int32_t back = 0;
    int32_t front = 0;

    const vec3*    vecs     = m_vertices.begin();
    const int32_t* inds     = indices.begin();
    const int32_t  indcount = indices.count();

    vec4 plane = CalculatePlane(
        inds[pivot], inds[pivot+1], inds[pivot+2]);

    for(int32_t i = 0; i + 2 < indcount; i += 3)
    {
        ivec3 side = Sign(vec3(
            Distance(plane, vecs[inds[i + 0]]),
            Distance(plane, vecs[inds[i + 1]]),
            Distance(plane, vecs[inds[i + 2]])
        ));

        switch(SplitType(side[0], side[1], side[2]))
        {
          case SplitType(-1, -1, -1):
          case SplitType(-1, -1,  0):
          case SplitType(-1,  0, -1):
          case SplitType(-1,  0,  0):
          case SplitType( 0, -1, -1):
          case SplitType( 0, -1,  0):
          case SplitType( 0,  0, -1):
            back++;
            break;

          case SplitType( 0,  0,  1):
          case SplitType( 0,  1,  0):
          case SplitType( 0,  1,  1):
          case SplitType( 1,  0,  0):
          case SplitType( 1,  0,  1):
          case SplitType( 1,  1,  0):
          case SplitType( 1,  1,  1):
            front++;
            break;

          case SplitType( 0,  0,  0):
            break;

          case SplitType(-1, -1,  1):
          case SplitType(-1,  1, -1):
          case SplitType( 1, -1, -1):
            back += 2;
            front++;
            break;

          case SplitType(-1,  0,  1):
          case SplitType( 1,  0, -1):
          case SplitType(-1,  1,  0):
          case SplitType( 1, -1,  0):
          case SplitType( 0, -1,  1):
          case SplitType( 0,  1, -1):
            back++;
            front++;
            break;

          case SplitType(-1,  1,  1):
          case SplitType( 1, -1,  1):
          case SplitType( 1,  1, -1):
            back++;
            front+=2;
            break;
        }
    }

    return ivec2(back, front);
}

BspTree::Node* BspTree::MakeTree(const Array<int32_t>& indices)
{
    if(indices.empty())
    {
        return nullptr;
    }
    
    int32_t best = 0;
    const bool search = true;
    if(search)
    {
        ivec2 pivot = EvaluatePivot(0, indices);
        for(int32_t i = 3; i + 2 < indices.count(); i += 3)
        {
            ivec2 newPivot = EvaluatePivot(i, indices);
            int32_t A = newPivot.x + newPivot.y;
            int32_t B = pivot.x + pivot.y;

            bool lt = A < B;
            bool eq = A == B;
            bool bd = abs(newPivot.x - newPivot.y) < abs(pivot.x - pivot.y);

            if(lt || (eq && bd))
            {
                best = i;
                pivot = newPivot;
            }
        }
    }
    else
    {
        best = Rand(0u, indices.count());
        best -= best % 3u;
    }

    Array<int32_t> back, front;
    Node* node  = m_nodes.Alloc();
    node->plane = SeparateTriangles(best, indices, back, front, node->triangles);
    node->back  = MakeTree(back);
    node->front = MakeTree(front);

    return node;
}

void BspTree::SortBackToFront(
    const vec3&         p, 
    const Node*         n, 
    Array<int32_t>&     out) const
{
    if(!n)
    {
        return;
    }

    if(Distance(n->plane, p) < 0.0f)
    {
        SortBackToFront(p, n->front, out);
        out.expand(n->triangles.count());
        for(int32_t i : n->triangles)
        {
            out.append() = i;
        }
        SortBackToFront(p, n->back, out);
    }
    else
    {
        SortBackToFront(p, n->back, out);
        out.expand(n->triangles.count());
        for(int32_t i : n->triangles)
        {
            out.append() = i;
        }
        SortBackToFront(p, n->front, out);
    }
}

void BspTree::SortFrontToBack(
    const vec3&         p, 
    const Node*         n, 
    Array<int32_t>&     out) const
{
    if(!n)
    {
        return;
    }

    if(Distance(n->plane, p) > 0.0f)
    {
        SortFrontToBack(p, n->front, out);
        out.expand(n->triangles.count());
        for(int32_t i : n->triangles)
        {
            out.append() = i;
        }
        SortFrontToBack(p, n->back, out);
    }
    else
    {
        SortFrontToBack(p, n->back, out);
        out.expand(n->triangles.count());
        for(int32_t i : n->triangles)
        {
            out.append() = i;
        }
        SortFrontToBack(p, n->front, out);
    }
}

bool BspTree::IsInside(const vec3& p, const Node* n) const
{
    if(Distance(n->plane, p) < 0.0f)
    {
        if(n->back)
        {
            return IsInside(p, n->back);
        }
        return true;
    }
    if(n->front)
    {
        return IsInside(p, n->front);
    }
    return false;
}

void BspTree::RecalculatePlanes(Node* n)
{
    if(!n)
    {
        return;
    }
    n->plane = CalculatePlane(
        n->triangles[0], 
        n->triangles[1], 
        n->triangles[2]);

    RecalculatePlanes(n->front);
    RecalculatePlanes(n->back);
}

bool BspTree::ClassifyTri(
    const Node*     n,
    const vec3&     v1,
    const vec3&     v2,
    const vec3&     v3,
    bool            inside,
    bool            keepEdge,
    Array<vec3>&    out,
    bool            keepNow) const 
{
    if(!n)
    {
        if(keepNow)
        {
            out.expand(3);
            out.append() = v1;
            out.append() = v2;
            out.append() = v3;
        }
        return keepNow;
    }

    vec3 dist(
        Distance(n->plane, v1),
        Distance(n->plane, v2),
        Distance(n->plane, v3)
    );

    ivec3 side = Sign(dist);

    FixedArray<vec3, 3> tmp;
    ivec3 A;

    if(side[0] * side[1] == -1)
    {
        A[0] = AppendInterpolate(
                tmp,
                v1,
                v2,
                Relation(dist[0], dist[1]));
    }
    if(side[1] * side[2] == -1)
    {
        A[1] = AppendInterpolate(
            tmp,
            v2,
            v3,
            Relation(dist[1], dist[2]));
    }
    if(side[2] * side[0] == -1)
    {
        A[2] = AppendInterpolate(
            tmp,
            v3,
            v1,
            Relation(dist[2], dist[0]));
    }

    int32_t preAddSize = out.count();
    bool complete = true;
    bool clipped = true;

    switch(SplitType(side[0], side[1], side[2]))
    {
        case SplitType(-1, -1, -1):
        case SplitType(-1, -1,  0):
        case SplitType(-1,  0, -1):
        case SplitType(-1,  0,  0):
        case SplitType( 0, -1, -1):
        case SplitType( 0, -1,  0):
        case SplitType( 0,  0, -1):
        complete = ClassifyTri(
            n->back, 
            v1, v2, v3, 
            inside, keepEdge, out, inside);
        clipped = false;
        break;

        case SplitType( 0,  0,  1):
        case SplitType( 0,  1,  0):
        case SplitType( 0,  1,  1):
        case SplitType( 1,  0,  0):
        case SplitType( 1,  0,  1):
        case SplitType( 1,  1,  0):
        case SplitType( 1,  1,  1):
        complete = ClassifyTri(
            n->front, 
            v1, v2, v3, 
            inside, keepEdge, out, !inside);
        clipped = false;
        break;

        case SplitType( 0,  0,  0):
        if (keepEdge)
        {
            out.expand(3);
            out.append() = v1;
            out.append() = v2;
            out.append() = v3;
            clipped = false;
        }
        break;

        case SplitType( 1, -1,  0):
        complete &= ClassifyTri(
            n->back,  
            v2, v3, tmp[A[0]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, v3, v1, tmp[A[0]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType(-1,  0,  1):
        complete &= ClassifyTri(
            n->back,  
            v1, v2, tmp[A[2]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v2, v3, tmp[A[2]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 0,  1, -1):
        complete &= ClassifyTri(
            n->back,  
            v3, v1, tmp[A[1]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v1, v2, tmp[A[1]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType(-1,  1,  0):
        complete &= ClassifyTri(
            n->back,  
            v3, v1, tmp[A[0]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v2, v3, tmp[A[0]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 1,  0, -1):
        complete &= ClassifyTri(
            n->back,  
            v2, v3, tmp[A[2]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v1, v2, tmp[A[2]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 0, -1,  1):
        complete &= ClassifyTri(
            n->back,  
            v1, v2, tmp[A[1]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v3, v1, tmp[A[1]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 1, -1, -1):
        complete &= ClassifyTri(
            n->front, 
            v1, tmp[A[0]], tmp[A[2]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->back,  
            v2, tmp[A[2]], tmp[A[0]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->back,  
            v2, v3,        tmp[A[2]], 
            inside, keepEdge, out, inside);
        break;

        case SplitType(-1,  1, -1):
        complete &= ClassifyTri(
            n->front, 
            v2, tmp[A[1]], tmp[A[0]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->back,  
            v3, tmp[A[0]], tmp[A[1]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->back,  
            v3, v1,        tmp[A[0]], 
            inside, keepEdge, out, inside);
        break;

        case SplitType(-1, -1,  1):
        complete &= ClassifyTri(
            n->front, 
            v3, tmp[A[2]], tmp[A[1]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->back,  
            v1, tmp[A[1]], tmp[A[2]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->back,  
            v1, v2,        tmp[A[1]], 
            inside, keepEdge, out, inside);
        break;

        case SplitType(-1,  1,  1):
        complete &= ClassifyTri(
            n->back,  
            v1, tmp[A[0]], tmp[A[2]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v2, tmp[A[2]], tmp[A[0]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->front, 
            v2, v3,        tmp[A[2]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 1, -1,  1):
        complete &= ClassifyTri(
            n->back,  
            v2, tmp[A[1]], tmp[A[0]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v1, tmp[A[0]], tmp[A[1]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->front, 
            v3, v1,        tmp[A[1]], 
            inside, keepEdge, out, !inside);
        break;

        case SplitType( 1,  1, -1):
        complete &= ClassifyTri(
            n->back,  
            v3, tmp[A[2]], tmp[A[1]], 
            inside, keepEdge, out, inside);
        complete &= ClassifyTri(
            n->front, 
            v1, tmp[A[1]], tmp[A[2]], 
            inside, keepEdge, out, !inside);
        complete &= ClassifyTri(
            n->front, 
            v1, v2,        tmp[A[1]], 
            inside, keepEdge, out, !inside);
        break;

        default:
        complete = false;
        break;
    }

    if(complete && clipped)
    {
        out.resize(preAddSize);
        out.expand(3);
        out.append() = v1;
        out.append() = v2;
        out.append() = v3;
    }

    return complete;
}

void BspTree::ClassifyTree(
    const Node*         tree, 
    const vec3*         verts, 
    bool                inside, 
    bool                keepEdge, 
    Array<vec3>&        out) const
{
    if(!tree)
    {
        return;
    }
    const int32_t* inds    = tree->triangles.begin();
    const int32_t  count   = tree->triangles.count();
    for(int32_t i = 0; i + 2 < count; i += 3)
    {
        ClassifyTri(
            m_root,
            verts[inds[i + 0]],
            verts[inds[i + 1]],
            verts[inds[i + 2]],
            inside, keepEdge, out, false);
    }
    ClassifyTree(
        tree->front, 
        verts, inside, keepEdge, out);
    ClassifyTree(
        tree->back, 
        verts, inside, keepEdge, out);
}

void BspTree::ToVertices(
    const vec3&     p, 
    Array<int32_t>& inds, 
    Array<Vertex>&  verts)
{
    inds.resize(m_vertices.count());
    for(int32_t i = 0; i < m_vertices.count(); ++i)
    {
        inds[i] = i;
    }

    PositionsToVertices(m_vertices, inds, verts);
}

BspTree& BspTree::Transform(const mat4& m)
{
    for(vec3& v : m_vertices)
    {
        v = vec3(m * vec4(v, 1.0f));
    }
    RecalculatePlanes(m_root);
    return *this;
}
