#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "allocator.h"
#include "window.h"
#include "camera.h"
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
#include "vertex.h"

Window window;
Camera camera;

void Init()
{
    SRand(time(0) ^ (uint64_t)&puts);
    window.Init("RL1", false);
    Window::SetActive(&window);
    camera.Init(window.m_width, window.m_height);
    camera.move(vec3(0.0f, 0.0f, -3.0f));
    Camera::SetActive(&camera);
    stm_setup();
    UI::Init();
    Components::Init();
    TaskManager::Init();
    Physics::Init();
    Renderer::Init();


    slot material = Images::Create("bumpy_PRMA");
    slot normal = Images::Create("bumpy_normal");

    {
        slot ent = Components::Create();
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));

        TempArray<vec3> pts;
        CSG csgs[] = 
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

        CSGUtil::Evaluate(csgs, NELEM(csgs), pts, vec3(0.0f), 3.0f, 128);

        TempArray<Vertex> verts;
        TempArray<int32_t> inds;
        PositionsToVertices(pts, verts, inds);

        Renderer::BufferDesc desc;
        desc.vertexData     = verts.begin();
        desc.vertexBytes    = verts.bytes();
        desc.indexData      = inds.begin();
        desc.indexBytes     = inds.bytes();
        desc.elementCount   = inds.count();

        rc->m_buffer    = Buffers::Create(desc);
        rc->m_material  = material;
        rc->m_normal    = normal;
    }
    
    {
        slot ent = Components::Create();
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);

        TempArray<vec3> pts;
        CSG csgs[] = 
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

        CSGUtil::Evaluate(csgs, NELEM(csgs), pts, vec3(0.0f), 3.0f, 128);

        TempArray<Vertex> verts;
        TempArray<int32_t> inds;
        PositionsToVertices(pts, verts, inds);

        Renderer::BufferDesc desc;
        desc.vertexData     = verts.begin();
        desc.vertexBytes    = verts.bytes();
        desc.indexData      = inds.begin();
        desc.indexBytes     = inds.bytes();
        desc.elementCount   = inds.count();

        rc->m_buffer    = Buffers::Create(desc);
        rc->m_material  = material;
        rc->m_normal    = normal;
        rc->m_matrix    = glm::translate(mat4(1.0f), vec3(5.0f, 0.0f, 0.0f));
    }
}
