#include "init.h"

#include "macro.h"
#include "window.h"
#include "camera.h"
#include "pipeline.h"
#include "shader.h"
#include "transform.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "task.h"

#include "sokol_gfx.h"
#include "sokol_time.h"

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
    window.Init("RL1", false);
    Window::SetActive(&window);
    camera.Init(window.m_width, window.m_height);
    Camera::SetActive(&camera);

    sg_desc desc = {0};
    sg_setup(&desc);
    stm_setup();

    Components::Init();
    TaskManager::Init();
    
    sg_shader_desc shadesc = {0};
    shadesc.vs.source = vs_src;
    shadesc.vs.uniform_blocks[0].size = sizeof(Transform);
    shadesc.vs.uniform_blocks[0].uniforms[0] = { "mvp", SG_UNIFORMTYPE_MAT4 };
    shadesc.fs.source = fs_src;
    shadesc.fs.images[0].name = "tex";
    shadesc.fs.images[0].type = SG_IMAGETYPE_2D;

    Shaders::Create("textured_static", shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = Shaders::Get(Shaders::Find("textured_static"));
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "uv0";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;
    
    slot pipeslot = Pipelines::Create("textured_static", pdesc);

    slot bufslot = Buffers::Load("triangle");
    slot imgslot = Images::Load("penta");
    
    slot ent = Components::Create();
    RenderComponent* rc = Components::GetAdd<RenderComponent>(ent);
    TransformComponent* xform = Components::GetAdd<TransformComponent>(ent);
    
    rc->m_buf = bufslot;
    rc->m_img = imgslot;
    rc->m_pipeline = pipeslot;
    
    xform->m_mat = mat4(1.0f);
}
