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

#include "imguishim.h"

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

    ImGuiShim::Init(window.m_window);

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
    shadesc.fs.uniform_blocks[0].uniforms[5] = { "RoughnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.uniform_blocks[0].uniforms[6] = { "MetalnessOffset", SG_UNIFORMTYPE_FLOAT  };
    shadesc.fs.images[0].name = "MatTex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;
    shadesc.fs.images[1].name = "PalTex";
    shadesc.fs.images[1].type = SG_IMAGETYPE_2D;

    slot shaderSlot = Shaders::Create(shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = Shaders::Get(shaderSlot);
    pdesc.index_type = SG_INDEXTYPE_UINT16;
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "normal";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[2].name = "uv0";
    pdesc.layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;
    
    slot pipeslot       = Pipelines::Create(pdesc);

    {
        slot ent = Components::Create();
        Components::Add<TransformComponent>(ent);
        RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
        PhysicsComponent* pc = Components::GetAdd<PhysicsComponent>(ent);

        pc->Init(0.0f, vec3(0.0f, 0.0f, 0.0f), vec3(10.0f, 0.33f, 10.0f));
        
        rc->m_buffer    = Buffers::Load(BufferString("sphere"));
        rc->m_material  = Images::Load(ImageString("bumpy_PHRM"));
        rc->m_palette   = Images::Load(ImageString("palette_palette"));
        rc->m_pipeline  = pipeslot;
    }
}
