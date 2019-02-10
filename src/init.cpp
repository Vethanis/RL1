#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "allocator.h"
#include "window.h"
#include "camera.h"
#include "control.h"
#include "shader.h"
#include "physics.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "rendercomponent.h"
#include "task.h"
#include "prng.h"
#include "csg.h"
#include "ui.h"
#include "sokol_time.h"
#include "renderer.h"
#include "vkrenderer.h"
#include "vertex.h"

#include "stb_perlin.h"

static Camera defaultCam;

void Init()
{
    SRand(time(0) ^ (uint64_t)&puts);
    
    Window::SetActive(Window::Init("RL1", false));
    int32_t wwidth = 0;
    int32_t wheight = 0;
    Window::GetSize(Window::GetActive(), wwidth, wheight);
    defaultCam.Init(wwidth, wheight);
    defaultCam.move(vec3(0.0f, 0.0f, -3.0f));
    Camera::SetActive(&defaultCam);
    
    stm_setup();
    UI::Init();
    Control::Init();
    Components::Init();
    TaskManager::Init();
    Physics::Init();

    Renderer::Init();

    Renderer::Texture material;
    Renderer::Texture normal;

    {
        TempArray<vec4> heights;
        const int32_t width = 1024;
        const int32_t height = 1024;
        const float pitch = 16.0f / width;
        heights.resize(width * height);
        float bumpScale = 10.0f;
        auto Sample = [&](float x, float y) -> float
        {
            return stb_perlin_noise3(
                pitch * x, pitch * y, 0.0f,
                16, 16, 16) * 0.5f + 0.5f;
        };
        for(int32_t y = 0; y < height; ++y)
        {
            for(int32_t x = 0; x < width; ++x)
            {
                heights[x + y * width] = vec4(Sample(x, y), 0.0f, 0.0f, 1.0f);
            }
        }
        TempArray<vec4> normals;
        normals.resize(width * height);
        bumpScale = 1.0f / bumpScale;
        for(int32_t y = 0; y < height; ++y)
        {
            for(int32_t x = 0; x < width; ++x)
            {
                float h0 = Sample(x, y);
                float h1 = Sample(x + 1, y);
                float h2 = Sample(x, y + 1);
                vec3 v01 = vec3(x + 1, y, h1 - h0);
                vec3 v02 = vec3(x, y + 1, h2 - h0);
                vec3 N = glm::cross(v01, v02);
                N.z *= bumpScale;
                N = glm::normalize(N);
                normals[x + y * width] = 0.5f * vec4(N, 0.0f) + 0.5f;
            }
        }

        Renderer::TextureDesc desc;
        MemZero(desc);
        desc.data       = heights.begin();
        desc.format     = Renderer::RGBA16F;
        desc.height     = height;
        desc.layers     = 1;
        desc.type       = Renderer::Texture2D;
        desc.width      = width;
        desc.magFilter  = Renderer::Linear;
        desc.minFilter  = Renderer::LinearMipmap;
        desc.wrapType   = Renderer::Repeat;
        material        = Renderer::CreateTexture(desc);
        desc.data       = normals.begin();
        normal          = Renderer::CreateTexture(desc);
    }

    {
        slot ent = Components::Create();
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));

        const CSG csgs[] = 
        {
            {
                vec3(0.0f),
                vec3(1.0f),
                0.1f,
                Box,
                Add,
            },
            {
                vec3(-0.5f),
                vec3(1.0f),
                0.1f,
                Sphere,
                SmoothAdd,
            },
            {
                vec3(0.5f),
                vec3(1.0f),
                0.1f,
                Sphere,
                Sub,
            }
        };

        TempArray<Vertex> pts;
        CSGUtil::Evaluate(csgs, NELEM(csgs), pts, vec3(0.0f), 3.0f, 128);

        TempArray<Vertex> verts;
        TempArray<int32_t> inds;
        IndexVertices(pts, verts, inds);

        Renderer::BufferDesc descs[2];
        MemZero(descs);
        descs[0].data       = verts.begin();
        descs[0].count      = verts.count();
        descs[0].stride     = sizeof(verts[0]);
        descs[0].type       = Renderer::Vertices;
        descs[1].data       = inds.begin();
        descs[1].count      = inds.count();
        descs[1].stride     = sizeof(inds[0]);
        descs[0].type       = Renderer::Indices;

        rc->m_type      = PT_Textured;
        rc->m_vertices  = Renderer::CreateBuffer(descs[0]);
        rc->m_indices   = Renderer::CreateBuffer(descs[1]);
        rc->m_material  = material;
        rc->m_normal    = normal;
        rc->m_matrix    = mat4(1.0f);
    }

    {
        slot ent = Components::Create();
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);

        const CSG csgs[] = 
        {
            {
                vec3(0.5f, 0.0f, 0.0f),
                vec3(2.0f),
                0.5f,
                Sphere,
                Add,
            },
            {
                vec3(0.5f, 0.0f, 0.0f),
                vec3(1.5f),
                0.5f,
                Box,
                Sub,
            },
        };

        TempArray<Vertex> pts;
        CSGUtil::Evaluate(csgs, NELEM(csgs), pts, vec3(0.0f), 3.0f, 128);

        TempArray<Vertex> verts;
        TempArray<int32_t> inds;
        IndexVertices(pts, verts, inds);

        Renderer::BufferDesc descs[2];
        MemZero(descs);
        descs[0].data       = verts.begin();
        descs[0].count      = verts.count();
        descs[0].stride     = sizeof(verts[0]);
        descs[0].type       = Renderer::Vertices;
        descs[1].data       = inds.begin();
        descs[1].count      = inds.count();
        descs[1].stride     = sizeof(inds[0]);
        descs[0].type       = Renderer::Indices;

        rc->m_type      = PT_Flat;
        rc->m_vertices  = Renderer::CreateBuffer(descs[0]);
        rc->m_indices   = Renderer::CreateBuffer(descs[1]);
        rc->m_matrix    = glm::translate(mat4(1.0f), vec3(5.0f, 0.0f, 0.0f));
    }
}
