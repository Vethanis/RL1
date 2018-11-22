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
    "in vec2 uv0;\n"
    "out vec2 uv;\n"
    "uniform mat4 mvp;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = mvp * vec4(position.xyz, 1.0);\n"
    "   uv = uv0;\n"
    "}\n";

const char fs_src[] = "#version 330 core\n"
    "in vec2 uv;\n"
    "out vec4 frag_color;\n"
    "uniform sampler2D tex;\n"
    "void main()\n"
    "{\n"
    "   frag_color = texture(tex, uv);"
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
    Physics::Init(1.0f / 60.0f);
    
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
    pdesc.layout.attrs[1].name = "uv0";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    
    slot pipeslot = Pipelines::Create("textured_static", pdesc);

    slot heightslot;
    if(false)
    {
        const uint32_t width = 20;
        const uint32_t height = 20;
        const float pitch = 0.5f;
        const float hpitch = 0.33f;
        float field[height][width];
        Vertex verts[height - 1][width - 1][6];
        for(uint32_t y = 0; y < height; ++y)
        {
            for(uint32_t x = 0; x < width; ++x)
            {
                field[y][x] = rand() / (float)RAND_MAX;
            }
        }
        for(uint32_t y = 0; y < height - 1; ++y)
        {
            for(uint32_t x = 0; x < width - 1; ++x)
            {
                Vertex* vs = verts[y][x];
                float pts[4][3] = {0};
                float uvs[4][2] = {0};
                pts[0][0] = (x + 0) * pitch;
                pts[0][2] = (y + 0) * pitch;
                pts[0][1] = field[y + 0][x + 0] * hpitch;
                uvs[0][0] = 0.0f;
                uvs[0][0] = 0.0f;
                pts[1][0] = (x + 1) * pitch;
                pts[1][2] = (y + 0) * pitch;
                pts[1][1] = field[y + 0][x + 1] * hpitch;
                uvs[1][0] = 1.0f;
                uvs[1][1] = 0.0f;
                pts[2][0] = (x + 1) * pitch;
                pts[2][2] = (y + 1) * pitch;
                pts[2][1] = field[y + 1][x + 1] * hpitch;
                uvs[2][0] = 1.0f;
                uvs[2][1] = 1.0f;
                pts[3][0] = (x + 0) * pitch;
                pts[3][2] = (y + 1) * pitch;
                pts[3][1] = field[y + 1][x + 0] * hpitch;
                uvs[3][0] = 0.0f;
                uvs[3][1] = 1.0f;

                const int32_t seq[] = { 0, 2, 1, 0, 3, 2 };
                for(int32_t i = 0; i < 6; ++i)
                {
                    memcpy(&vs[i].position, pts[seq[i]], sizeof(vec3));
                    memcpy(&vs[i].uv, uvs[seq[i]], sizeof(vec2));
                }
            }
        }
        heightslot = Buffers::Create("heightfield", &verts[0][0][0], (height - 1) * (width - 1) * 6);
    }


    {
        CSG csgs[2];
        csgs[0].size = vec3(2.0f, 0.1f, 2.0f);
        csgs[0].shape = Box;
        csgs[0].blend = SmoothAdd;
        csgs[0].smoothness = 0.7f;

        csgs[1].size = vec3(1.0f);
        csgs[1].shape = Sphere;
        csgs[1].blend = SmoothAdd;
        csgs[1].smoothness = 0.7f;

        CSGList csglist;
        csglist.indices.grow() = 0;
        csglist.indices.grow() = 1;

        Array<vec3> pts;
        Array<Vertex> verts;

        float pitch = CreatePoints(csglist, csgs, 7, vec3(0.0f), 5.0f, pts);
        PointsToCubes(pts, pitch, verts);

        heightslot = Buffers::Create("heightfield", verts.begin(), verts.count());
    }


    slot bufslot = Buffers::Load("triangle");
    slot imgslot = Images::Load("dirt");
    
    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(10.0f, vec3(0.0f, 10.0f, 0.0f), vec3(1.0f));
        
        rc->m_buf = bufslot;
        rc->m_img = imgslot;
        rc->m_pipeline = pipeslot;
    }
    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));
        
        rc->m_buf = heightslot;
        rc->m_img = imgslot;
        rc->m_pipeline = pipeslot;
    }
}
