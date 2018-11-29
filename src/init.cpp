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
#include "prng.h"

#include "sokol_gfx.h"
#include "sokol_time.h"

#include "shaders/textured.h"

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

    Components::Init();
    TaskManager::Init();
    Physics::Init();
    
    sg_shader_desc shadesc = {0};

    shadesc.vs.source = textured_vs;
    shadesc.vs.uniform_blocks[0].size = sizeof(VSUniform);
    shadesc.vs.uniform_blocks[0].uniforms[0] = { "MVP", SG_UNIFORMTYPE_MAT4 };
    shadesc.vs.uniform_blocks[0].uniforms[1] = { "M",   SG_UNIFORMTYPE_MAT4 };

    shadesc.fs.source = textured_fs;
    shadesc.fs.uniform_blocks[0].size = sizeof(FSUniform);
    shadesc.fs.uniform_blocks[0].uniforms[0] = { "Eye",             SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[1] = { "LightDir",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[2] = { "LightRad",        SG_UNIFORMTYPE_FLOAT3 };
    shadesc.fs.uniform_blocks[0].uniforms[3] = { "BumpScale",       SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[4] = { "ParallaxScale",   SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.images[0].name = "MatTex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;
    shadesc.fs.images[1].name = "PalTex";
    shadesc.fs.images[1].type = SG_IMAGETYPE_2D;

    slot shaderSlot = Shaders::Create("textured_static", shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = Shaders::Get(shaderSlot);
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "normal";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[2].name = "uv0";
    pdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;

    uint32_t palette[256];
    vec4 colors[4];
    memset(palette, 0, sizeof(palette));
    for(vec4& c : colors)
    {
        c = vec4(Randf(), Randf(), Randf(), 1.0f) * 254.0f;
    }
    const int32_t palPerCol = NELEM(palette) / NELEM(colors);
    for(int32_t pal = 0; pal < NELEM(palette); ++pal)
    {
        for(int32_t col = 0; col < NELEM(colors); ++col)
        {
            int32_t pt = col * palPerCol;
            int32_t dis = (int32_t)sqrtf((pal - pt) * (pal - pt));
            if(dis < palPerCol)
            {
                uint32_t& x = palette[pal];
                const float alpha = 1.0f - (float)dis / (float)palPerCol;
                vec4 c = colors[col] * alpha;
                uint8_t r = x & 0xff;
                uint8_t g = (x & 0xff00) >> 8;
                uint8_t b = (x & 0xff0000) >> 16;
                uint8_t a = (x & 0xff000000) >> 24;
                r += (uint8_t)c.x;
                g += (uint8_t)c.y;
                b += (uint8_t)c.z;
                a += (uint8_t)c.w;
                x = a;
                x = (x << 8) | b;
                x = (x << 8) | g;
                x = (x << 8) | r;
            }
        }
    }

    
    slot pipeslot       = Pipelines::Create("textured_static", pdesc);
    slot matSlot        = Images::Load("bumpy");
    slot paletteSlot    = Images::Create("rgb", palette, NELEM(palette), 1);

    const float verts[] = 
    {
    //  position                  normal              uv
        -0.5f, -0.5f,  0.0f,      0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
         0.0f,  0.5f,  0.0f,      0.0f, 0.0f, 1.0f,   0.0f, 1.0f,
         0.5f, -0.5f,  0.0f,      0.0f, 0.0f, 1.0f,   1.0f, 0.0f,
    };
    //Buffers::Save("triangle", (const Vertex*)verts, 3);
    slot terrainSlot = Buffers::Load("triangle");

    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));
        
        rc->m_buf = terrainSlot;
        rc->m_material = matSlot;
        rc->m_palette = paletteSlot;
        rc->m_pipeline = pipeslot;
    }
}
