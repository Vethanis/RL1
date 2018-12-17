
#include "vertex.h"

void PositionsToVertices(const Array<vec3>& verts, const Array<int32_t>& inds, Array<Vertex>& out)
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
