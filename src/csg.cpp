#include "csg.h"

#include "macro.h"
#include "task.h"
#include "sema.h"

struct Hit
{
    struct CSGContext*  m_context;
    vec3                m_center;
    float               m_radius;
    int32_t             m_depth;
};

struct CSGContext
{
    Array<vec3>*    output;
    const CSG*      csgs;
    const CSGList*  list;
    int32_t         max_depth;
    std::mutex      outputMutex;

    void Start(vec3 center, float radius)
    {
        output->clear();

        float hr = radius / 5.0f;
        float pitch = hr * 2.0f;
        for(int32_t z = -2; z <= 2; ++z)
        {
            for(int32_t y = -2; y <= 2; ++y)
            {
                for(int32_t x = -2; x <= 2; ++x)
                {
                    vec3 c = center + vec3(x, y, z) * pitch;
                    Hit newHit;
                    newHit.m_context = this;
                    newHit.m_center = c;
                    newHit.m_radius = hr;
                    newHit.m_depth = 0;

                    Task task;
                    task.fn = SAct;
                    memcpy(task.mem + 0, &newHit, sizeof(Hit));

                    TaskManager::Add(TT_MeshGen, task);
                }
            }
        }

        TaskManager::Start(TT_MeshGen);
    }
    static void SAct(Task* task)
    {
        Hit hit;
        memcpy(&hit, task->mem + 0, sizeof(Hit));
        hit.m_context->Act(hit);
    }
    void Act(const Hit& hit)
    {
        maphit mh = list->Map(hit.m_center, csgs);

        if(fabsf(mh.distance) > hit.m_radius * 1.732051f)
        {
            return;
        }

        if(hit.m_depth == max_depth)
        {
            LockGuard guard(outputMutex);
            output->grow() = hit.m_center;
            return;
        }

        const float hr = hit.m_radius * 0.5f;
        for(int32_t i = 0; i < 8; i++)
        {
            vec3 c(hit.m_center);
            c.x += (i & 4) ? hr : -hr;
            c.y += (i & 2) ? hr : -hr;
            c.z += (i & 1) ? hr : -hr;

            Hit newHit;
            newHit.m_context = this;
            newHit.m_center = c;
            newHit.m_radius = hr;
            newHit.m_depth = hit.m_depth + 1;

            Task task;
            task.fn = SAct;
            memcpy(task.mem + 0, &newHit, sizeof(Hit));
            TaskManager::Add(TT_MeshGen, task);
        }
    }
};

float CreatePoints(
    const CSGList&  list, 
    const CSG*      csgs, 
    int32_t         max_depth, 
    vec3            center, 
    float           radius, 
    Array<vec3>&    out)
{
    CSGContext ctx;
    ctx.list = &list;
    ctx.csgs = csgs;
    ctx.max_depth = max_depth;
    ctx.output = &out;

    ctx.Start(center, radius);
    
    const float pitch = (radius / 5.0f) / (float)(1 << (max_depth - 1));
    return pitch;
}

struct CubeVert
{
    vec3 position;
    vec3 normal;
    vec2 uv;
};

static const float cube[] = 
{
//  pos                     normal          uvs
    -0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
     0.5f, -0.5f, -0.5f,    0.0f, 0.0f, -1.0f,  1.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,  1.0f, 1.0f,
    -0.5f,  0.5f, -0.5f,    0.0f, 0.0f, -1.0f,  0.0f, 1.0f,

    -0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,   0.0f, 0.0f, 
     0.5f, -0.5f,  0.5f,    0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,   1.0f, 1.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 0.0f, 1.0f,   0.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
    -0.5f,  0.5f, -0.5f,    -1.0f, 0.0f, 0.0f,  1.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,    -1.0f, 0.0f, 0.0f,  0.0f, 1.0f,

     0.5f, -0.5f, -0.5f,    1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
     0.5f,  0.5f, -0.5f,    1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,    1.0f, 0.0f, 0.0f,   0.0f, 1.0f,

    -0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
    -0.5f, -0.5f,  0.5f,    0.0f, -1.0f, 0.0f,  1.0f, 0.0f,
     0.5f, -0.5f,  0.5f,    0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,    0.0f, -1.0f, 0.0f,  0.0f, 1.0f,

    -0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
    -0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 0.0f,
     0.5f,  0.5f,  0.5f,    0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
     0.5f,  0.5f, -0.5f,    0.0f, 1.0f, 0.0f,   0.0f, 1.0f
};

static const int32_t indices[] = 
{
    0,  1,  2,      0,  2,  3,
    6,  5,  4,      7,  6,  4,
    8,  9,  10,     8,  10, 11,
    14, 13, 12,     15, 14, 12,
    16, 17, 18,     16, 18, 19,
    22, 21, 20,     23, 22, 20
};

void PointsToCubes(
    const Array<vec3>&  input, 
    float               pitch, // 2x the hit radius of max depth above
    const CSGList&      list, 
    const CSG*          csgs, 
    Array<Vertex>&      output)
{
    output.clear();
    output.reserve(input.count() * NELEM(indices));

    const CubeVert* cvs = (const CubeVert*)cube;
    for(const vec3& pt : input)
    {
        for(int32_t i : indices)
        {
            vec3 neighbor = pt + cvs[i].normal * pitch;

            maphit mh = list.Map(neighbor, csgs);
            if(fabsf(mh.distance) <= pitch * 0.5f * 1.732051f)
            {
                continue;
            }

            Vertex& vert = output.append();
            vert.position = pt;
            vert.position += pitch * cvs[i].position;
            vert.normal = cvs[i].normal;
            vert.uv = cvs[i].uv;

            float dis;
            do
            {
                vert.normal = list.Normal(vert.position, csgs);
                dis = list.Map(vert.position, csgs).distance;
                vert.position -= dis * vert.normal;
            }
            while(fabsf(dis) > 0.001f);

            vert.ao = 0.0f; //list.AO(vert.position, vert.normal, csgs, pitch);
        }
    }
}
