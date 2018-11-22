#pragma once

#include "linmath.h"
#include "array.h"

enum Shape : uint8_t
{
    Sphere = 0,
    Box,
    ShapeCount
};

enum Blend : uint8_t
{
    Add = 0,
    Sub,
    SmoothAdd,
    SmoothSub,
    BlendCount
};

struct maphit 
{
    uint32_t    id;
    float       distance;
};

inline float operator-(maphit a, maphit b)
{
    return a.distance - b.distance;
}

inline float operator-(float a, maphit b)
{
    return a - b.distance;
}

inline float operator-(maphit a, float b)
{
    return a.distance - b;
}

struct CSG
{
    vec3    center;
    vec3    size;
    float   smoothness;
    Shape   shape;
    Blend   blend;

    inline float Sphere(const vec3& p) const 
    {
        return glm::distance(center, p) - size.x;
    }
    inline float Box(const vec3& p) const 
    {
        vec3 d = glm::abs(p - center) - size;
        return glm::min(glm::max(d.x, glm::max(d.y, d.z)), 0.0f) + 
            glm::length(glm::max(d, vec3(0.0f)));
    }
    inline maphit Add(maphit a, maphit b) const
    {
        return (a.distance < b.distance) ? a : b;
    }
    inline maphit Sub(maphit a, maphit b) const
    {
        b.distance = -b.distance;
        return (a.distance > b.distance) ? a : b;
    }
    inline maphit SmoothAdd(maphit a, maphit b) const
    {
        float e = glm::max(smoothness - glm::abs(a.distance - b.distance), 0.0f);
        float dis = glm::min(a.distance, b.distance) - e * e * 0.25f / smoothness;
        return 
        {
            a.distance < b.distance ? a.id : b.id, 
            dis
        };
    }
    inline maphit SmoothSub(maphit a, maphit b) const
    {
        a.distance = -a.distance;
        maphit m = SmoothAdd(a, b);
        m.distance = -m.distance;
        return m;
    }
    inline float Distance(const vec3& p) const 
    {
        switch(shape)
        {
            default:
            case Shape::Sphere: return Sphere(p);
            case Shape::Box: return Box(p);
        }
    }
    inline maphit Blend(maphit a, maphit b) const 
    {
        switch(blend)
        {
            default:
            case Blend::Add: return Add(a, b);
            case Blend::Sub: return Sub(a, b);
            case Blend::SmoothAdd: return SmoothAdd(a, b);
            case Blend::SmoothSub: return SmoothSub(a, b);
        }
    }
    inline vec3 Normal(const vec3& p) const
    {
        constexpr float e = 0.001f;
        return glm::normalize(vec3(
            Distance(p + vec3(e, 0.0f, 0.0f)) - Distance(p - vec3(e, 0.0f, 0.0f)),
            Distance(p + vec3(0.0f, e, 0.0f)) - Distance(p - vec3(0.0f, e, 0.0f)),
            Distance(p + vec3(0.0f, 0.0f, e)) - Distance(p - vec3(0.0f, 0.0f, e))
        ));
    }
};

struct CSGList 
{
    Array<uint32_t> indices;
    
    maphit Map(const vec3& p, const CSG* csgs) const 
    {
        maphit a = { 0xffffffff, FLT_MAX };
        for(uint32_t i : indices)
        {
            maphit b = 
            { 
                i, 
                csgs[i].Distance(p) 
            };
            a = csgs[i].Blend(a, b);
        }
        return a;
    }
    vec3 Normal(const vec3& p, const CSG* csgs) const 
    {
        constexpr float e = 0.001f;
        return glm::normalize(vec3(
            Map(p + vec3(e, 0.0f, 0.0f), csgs) - Map(p - vec3(e, 0.0f, 0.0f), csgs),
            Map(p + vec3(0.0f, e, 0.0f), csgs) - Map(p - vec3(0.0f, e, 0.0f), csgs),
            Map(p + vec3(0.0f, 0.0f, e), csgs) - Map(p - vec3(0.0f, 0.0f, e), csgs)
        ));
    }
};

#include "task.h"
#include "sema.h"

struct Hit
{
    vec3    m_center;
    float   m_radius;
    int32_t m_depth;
};

struct CSGContext
{
    Array<vec3>*    output;
    const CSG*      csgs;
    const CSGList*  list;
    int32_t         max_depth;
    Array<Hit>      hits;
    std::mutex      hitMutex;
    std::mutex      outputMutex;

    void Start(vec3 center, float radius)
    {
        output->clear();
        hits.clear();

        hits.grow() = 
        {
            center,
            radius, 
            0
        };

        for(int32_t i = 0; i < 8; ++i)
        {
            Act();
        }

        Task task;
        task.fn = SActLoop;
        memcpy(task.mem + 0, this, sizeof(size_t));

        TaskManager::Add(TT_General, task);
        TaskManager::Start(TT_General, 1);
    }
    static void SActLoop(Task* task)
    {
        CSGContext* ctx;
        memcpy(&ctx, task->mem + 0, sizeof(size_t));
        ctx->ActLoop();
    }
    void ActLoop()
    {
        for(int32_t checks = 0; checks < 10; ++checks)
        {
            bool hasWork = true;
            do
            {
                hasWork = Act();
            }
            while(hasWork);
        }
    }
    bool Act()
    {
        Hit hit;

        {
            LockGuard guard(hitMutex);
            if(hits.count() == 0)
            {
                return false;
            }

            Hit hit = hits.back();
            hits.pop();
        }
        
        maphit mh = list->Map(hit.m_center, csgs);

        if(fabsf(mh.distance) > hit.m_radius * 1.732051f)
        {
            return true;
        }

        if(hit.m_depth == max_depth)
        {
            LockGuard guard(outputMutex);
            output->grow() = hit.m_center;
            return true;
        }

        const float hr = hit.m_radius * 0.5f;
        for(int32_t i = 0; i < 8; i++)
        {
            vec3 c(hit.m_center);
            c.x += (i & 4) ? hr : -hr;
            c.y += (i & 2) ? hr : -hr;
            c.z += (i & 1) ? hr : -hr;

            Hit newHit;
            newHit.m_center = c;
            newHit.m_radius = hr;
            newHit.m_depth = hit.m_depth + 1;

            LockGuard guard(hitMutex);
            hits.grow() = newHit;
        }

        return true;
    }
};


template<int32_t fill_depth>
static void Fill(
    Array<vec3>&    output, 
    const CSG*      csgs, 
    const CSGList&  list, 
    const vec3&     center, 
    float           radius)
{
    output.clear();


    auto DoOne = [&]()
    {
        
    };

    auto Fn = [&]()
    {
        for(int32_t i = 0; i < 10; ++i)
        {
            while(true)
            {
                DoOne();
            }
        }
    };
    
}
