
#include "vertex.h"

void IndexVertices(
    const TempArray<Vertex>&    verts, 
    TempArray<Vertex>&          out, 
    TempArray<int32_t>&         indout)
{
    out.clear();
    indout.clear();
    indout.reserve(verts.count());
    out.reserve(verts.count() / 6);
    for(const Vertex& v : verts)
    {
        int32_t idx = -1;
        for(int32_t i = out.count() - 1; i >= 0; --i)
        {
            if(DISTSQ(v.position, out[i].position) == 0.0f)
            {
                idx = i;
                break;
            }
        }
        if(idx == -1)
        {
            idx = out.count();
            out.grow() = v;
        }
        indout.append() = idx;
    }
}

void PositionsToVertices(
    const TempArray<vec3>&      verts, 
    const TempArray<int32_t>&   inds, 
    TempArray<Vertex>&          out)
{
    out.clear();
    out.resize(verts.count());

    for(int32_t i = 0; i < verts.count(); ++i)
    {
        out[i].position = verts[i];
        out[i].normal = vec3(0.0f);
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
}
