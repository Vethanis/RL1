#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "allocator.h"
#include "window.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "physics.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "rendercomponent.h"
#include "task.h"
#include "prng.h"

#include "csg.h"

#include "imguishim.h"

#include "sokol_gfx.h"
#include "sokol_time.h"

#include "shaders/textured.h"
#include "shaders/sky.h"

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

    sg_desc desc = {0};
    sg_setup(&desc);
    stm_setup();

    ImGuiShim::Init(window.m_window);

    Components::Init();
    TaskManager::Init();
    Physics::Init();
    
    Shaders::Create(ST_Textured, Textured::GetShaderDesc());
    Shaders::Create(ST_Sky, Sky::GetShaderDesc());

    Pipelines::Create(PT_Textured, Textured::GetPipelineDesc());
    Pipelines::Create(PT_Sky, Sky::GetPipelineDesc());

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
                0.5f,
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

        CSGUtil::Evaluate(csgs, NELEM(csgs), pts, vec3(0.0f), 3.0f, 64);

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
        rc->m_pipeline  = PT_Textured;
    }

    {
        slot ent = Components::Create();
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        rc->m_matrix = mat4(1.0f);
        rc->m_pipeline = PT_Sky;

        vec2 verts[] = 
        {
            vec2(-1.0f, 3.0f),
            vec2(-1.0f, -1.0f),
            vec2(3.0f, -1.0f),
        };
        BufferData bd;
        memset(&bd, 0, sizeof(bd));
        bd.vertCount = NELEM(verts);
        bd.vertices = (void*)verts;
        bd.size = sizeof(verts[0]);
        rc->m_buffer = Buffers::Create(bd);
    }
}
