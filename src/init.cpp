#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "window.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "transform.h"
#include "physics.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "task.h"

#include "sokol_gfx.h"
#include "sokol_time.h"

#include "csg.h"


Window window;
Camera camera;

const char vs_src[] = "#version 330 core\n"
    "in vec3 position;\n"
    "in vec3 normal;\n"
    "in vec2 uv0;\n"
    "in float ao0;\n"
    "out vec3 N;\n"
    "out vec2 uv;\n"
    "out float AO;\n"
    "uniform mat4 mvp;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = mvp * vec4(position.xyz, 1.0);\n"
    "   N = normal;\n"
    "   uv = uv0;\n"
    "   AO = ao0;\n"
    "}\n";

const char fs_src[] = "#version 330 core\n"
    "in vec3 N;\n"
    "in vec2 uv;\n"
    "in float AO;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D tex;\n"
    "void main()\n"
    "{\n"
    "   vec3 L = normalize(vec3(1.0, 2.0f, 0.0f));\n"
    "   vec4 C = texture(tex, uv);\n"
    "   float D = max(0.0, dot(L, N));\n"
    "   float amb = 0.25;\n"
    "   C *= mix(amb, 1.0, D);\n"
    "   C *= mix(1.0, amb, AO);\n"
    "   frag_color = C;\n"
    "}\n";

void Init()
{
    srand((uint32_t)time(0));

    window.Init("RL1", false);
    Window::SetActive(&window);
    camera.Init(window.m_width, window.m_height);
    camera.move(vec3(0.0f, 0.0f, -3.0f));
    Camera::SetActive(&camera);

    sg_desc desc = {0};
    sg_setup(&desc);
    stm_setup();

    Components::Init();
    TaskManager::Init();
    Physics::Init();
    
    sg_shader_desc shadesc = {0};
    shadesc.vs.source = vs_src;
    shadesc.vs.uniform_blocks[0].size = sizeof(Transform);
    shadesc.vs.uniform_blocks[0].uniforms[0] = { "mvp", SG_UNIFORMTYPE_MAT4 };
    shadesc.fs.source = fs_src;
    shadesc.fs.images[0].name = "tex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;

    slot shaderSlot = Shaders::Create("textured_static", shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = Shaders::Get(shaderSlot);
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "normal";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[2].name = "uv0";
    pdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.layout.attrs[3].name = "ao0";
    pdesc.layout.attrs[3].format = SG_VERTEXFORMAT_FLOAT;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    
    slot pipeslot = Pipelines::Create("textured_static", pdesc);
    slot imgslot = Images::Load("dirt");

    slot terrainSlot;
    {
        CSG csgs[3];
        csgs[0].size = vec3(0.0f, 1.0f, 0.0f);
        csgs[0].shape = Plane;
        csgs[0].blend = SmoothAdd;
        csgs[0].smoothness = 0.75f;

        csgs[1].size = vec3(2.0f);
        csgs[1].shape = Sphere;
        csgs[1].blend = SmoothAdd;
        csgs[1].smoothness = 0.75f;

        CSGList csglist;
        csglist.indices.grow() = 0;
        csglist.indices.grow() = 1;

        Array<vec3> pts;
        Array<Vertex> verts;

        float pitch = CreatePoints(csglist, csgs, 5, vec3(0.0f), 4.0f, pts);
        PointsToCubes(pts, pitch, csglist, csgs, verts);

        terrainSlot = Buffers::Create("heightfield", verts.begin(), verts.count());
    }

    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));
        
        rc->m_buf = terrainSlot;
        rc->m_img = imgslot;
        rc->m_pipeline = pipeslot;
    }
}
