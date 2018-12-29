
#include "vertex.h"

void PositionsToVertices(
    const Array<vec3>&  verts, 
    Array<Vertex>&      out, 
    Array<int32_t>&     indout)
{
    BucketScopeStack scope;

    out.clear();
    indout.clear();

    Array<vec3> uniques;
    uniques.reserve(verts.count());
    for(const vec3& v : verts)
    {
        int32_t idx = -1;
        for(int32_t i = uniques.count() - 1; i >= 0; --i)
        {
            if(DISTSQ(v, uniques[i]) == 0.0f)
            {
                idx = i;
                break;
            }
        }
        if(idx == -1)
        {
            idx = uniques.count();
            uniques.append() = v;
        }
        indout.grow() = idx;
    }

    out.resize(uniques.count());
    for(int32_t i = 0; i < uniques.count(); ++i)
    {
        Vertex& vt = out[i];
        vt.position = uniques[i];
        vt.normal = vec3(0.00001f);
        vt.uv = vec2(0.0f);
    }

    for(int32_t i = 0; i + 2 < indout.count(); i += 3)
    {
        const int32_t a = indout[i + 0];
        const int32_t b = indout[i + 1];
        const int32_t c = indout[i + 2];
        const vec3 e1 = uniques[b] - uniques[a];
        const vec3 e2 = uniques[c] - uniques[a];
        vec3 N = glm::normalize(glm::cross(e1, e2));
        out[a].normal += N;
        out[b].normal += N;
        out[c].normal += N;
    }

    for(Vertex& v : out)
    {
        v.normal = glm::normalize(v.normal);

        vec3 n = glm::abs(v.normal);
        float m = CMAX(n);
        if(m == n.x)
        {
            v.uv.x = glm::sign(v.normal.x) * -v.position.z;
            v.uv.y = v.position.y;
        }
        else if(m == n.y)
        {
            v.uv.x = v.position.x;
            v.uv.y = glm::sign(v.normal.y) * -v.position.z;
        }
        else
        {
            v.uv.x = glm::sign(v.normal.z) * v.position.x;
            v.uv.y = v.position.y;
        }
    }
}

void PositionsToVertices(
    const Array<vec3>&      verts, 
    const Array<int32_t>&   inds, 
    Array<Vertex>&          out)
{
    out.clear();
    out.resize(verts.count());

    for(int32_t i = 0; i < verts.count(); ++i)
    {
        out[i].position = verts[i];
        out[i].normal = vec3(0.0f);
        out[i].uv = vec2(0.0f);
    }

    for(int32_t i = 0; i + 2 < inds.count(); i += 3)
    {
        int32_t a = inds[i + 0];
        int32_t b = inds[i + 1];
        int32_t c = inds[i + 2];
        vec3 e1 = verts[b] - verts[a];
        vec3 e2 = verts[c] - verts[a];
        vec3 N = glm::normalize(glm::cross(e1, e2));
        out[a].normal += N;
        out[b].normal += N;
        out[c].normal += N;
    }

    for(Vertex& v : out)
    {
        v.normal = glm::normalize(v.normal);

        vec3 n = glm::abs(v.normal);
        float m = CMAX(n);
        if(m == n.x)
        {
            v.uv.x = glm::sign(v.normal.x) * -v.position.z;
            v.uv.y = v.position.y;
        }
        else if(m == n.y)
        {
            v.uv.x = v.position.x;
            v.uv.y = glm::sign(v.normal.y) * -v.position.z;
        }
        else
        {
            v.uv.x = glm::sign(v.normal.z) * v.position.x;
            v.uv.y = v.position.y;
        }
    }
}
