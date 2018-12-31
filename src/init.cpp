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

        BufferData bd;
        bd.vertices     = verts.begin();
        bd.vertCount    = verts.count();
        bd.indices      = (uint32_t*)inds.begin();
        bd.indexCount   = inds.count();
        bd.size         = sizeof(verts[0]);

        rc->m_buffer    = Buffers::Create(bd);
        rc->m_material  = Images::Create("bumpy_PRMA");
        rc->m_normal    = Images::Create("bumpy_normal");
    }
}
