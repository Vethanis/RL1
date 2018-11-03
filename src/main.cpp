
#include <math.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

#include "sokol_gfx.h"
#include "sokol_time.h"

#include "macro.h"
#include "camera.h"
#include "window.h"

struct Uniforms
{
    uint8_t mvp[sizeof(mat4)];
};

struct context
{
    sg_buffer       buffer;
    sg_shader       shader;
    sg_pipeline     pipe;
    sg_pass_action  pass_action;
    sg_draw_state   drawstate;

    Window          window;
    Camera          cam;
    Uniforms        unis;
};

context ctx = { 0 };

void init(void)
{
    ctx.window.Init("RL1", false);

    sg_desc desc = {0};
    sg_setup(&desc);
    stm_setup();

    ctx.pass_action.colors[0] = { SG_ACTION_CLEAR, { 0.0f, 0.0f, 0.0f, 1.0f } };

    const float verts[] = 
    {
        // positions            // colors
         0.0f,  0.5f, 0.5f,     1.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.5f,     0.0f, 1.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f,     0.0f, 0.0f, 1.0f, 1.0f 
    };

    const char vs_src[] = "#version 330\n"
        "in vec3 position;\n"
        "in vec4 color0;\n"
        "out vec4 color;\n"
        "uniform mat4 mvp;\n"
        "void main()\n"
        "{\n"
        "   gl_Position = mvp * vec4(position.xyz, 1.0);\n"
        "   color = color0;\n"
        "}\n";
    
    const char fs_src[] = "#version 330\n"
        "in vec4 color;\n"
        "out vec4 frag_color;\n"
        "void main()\n"
        "{\n"
        "   frag_color = color;"
        "}\n";

    sg_buffer_desc bufdesc = {0};
    bufdesc.size = sizeof(verts);
    bufdesc.content = verts;
    ctx.buffer = sg_make_buffer(&bufdesc);

    sg_shader_desc shadesc = {0};
    shadesc.vs.source = vs_src;
    shadesc.vs.uniform_blocks[0].size = sizeof(Uniforms);
    shadesc.vs.uniform_blocks[0].uniforms[0] = { "mvp", SG_UNIFORMTYPE_MAT4 };
    shadesc.fs.source = fs_src;
    ctx.shader = sg_make_shader(&shadesc);

    sg_pipeline_desc pdesc = {0};
    pdesc.shader = ctx.shader;
    pdesc.layout.attrs[0].name = "position";
    pdesc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    pdesc.layout.attrs[1].name = "color0";
    pdesc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT4;
    pdesc.depth_stencil.depth_write_enabled = true;
    pdesc.depth_stencil.depth_compare_func = SG_COMPAREFUNC_LESS_EQUAL;
    pdesc.rasterizer.cull_mode = SG_CULLMODE_BACK;
    pdesc.rasterizer.face_winding = SG_FACEWINDING_CCW;
    ctx.pipe = sg_make_pipeline(&pdesc);

    ctx.drawstate.pipeline = ctx.pipe;
    ctx.drawstate.vertex_buffers[0] = ctx.buffer;
}

void shutdown(void)
{
    sg_shutdown();
    ctx.window.Shutdown();
}

void frame(void)
{
    ctx.window.Poll(ctx.cam);
    mat4 VP = ctx.cam.update();
    memcpy(ctx.unis.mvp, &VP, sizeof(mat4));

    sg_begin_default_pass(
        &ctx.pass_action, 
        ctx.window.m_width,
        ctx.window.m_height);
    {
        sg_apply_draw_state(&ctx.drawstate);
        sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &ctx.unis, sizeof(Uniforms));
        sg_draw(0, 3, 1);
    }
    sg_end_pass();
    sg_commit();
    ctx.window.Swap();
}

int main()
{
    init();
    while(ctx.window.Open())
    {
        frame();
    }
    shutdown();
}
