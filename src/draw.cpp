#include "draw.h"

#include "macro.h"
#include "component.h"
#include "rendercomponent.h"
#include "buffer.h"
#include "image.h"
#include "pipeline.h"
#include "window.h"
#include "camera.h"
#include "prng.h"

#include "shaders/textured.h"
#include "shaders/sky.h"

#include "sokol_gfx.h"
#include "uidraw.h"
#include "imgui.h"

sg_pass_action action = {0};
Textured::VSUniform texvsuni;
Textured::FSUniform texfsuni = 
{
    vec3(0.0f),
    glm::normalize(vec3(1.0f)),
    vec3(1.0f, 0.0f, 0.0f),
    vec3(0.0f, 1.0f, 0.0f),
    vec3(0.0f, 0.0f, 1.0f),
    0.5f,
    65.0f,
    0.0f,
    0.0f
};
Sky::VSUniform skyvsuni;
Sky::FSUniform skyfsuni;
Window* window;
Camera* cam;
mat4    VP;

void DrawTextured(const RenderComponent* rc)
{
    const Buffer*       buf         = Buffers::Get(rc->m_buffer);
    const sg_image*     material    = Images::Get(rc->m_material);
    const sg_image*     normal      = Images::Get(rc->m_normal);

    if(!buf || !material || !normal)
    {
        return;
    }

    texvsuni.MVP   = VP * rc->m_matrix;
    texvsuni.M     = rc->m_matrix;
    texfsuni.Seed  = Randf();

    sg_draw_state state     = { 0 };
    state.pipeline          = Pipelines::Get(PT_Textured);
    state.vertex_buffers[0] = buf->m_vertices;
    state.index_buffer      = buf->m_indices;
    state.fs_images[0]      = *material;
    state.fs_images[1]      = *normal;

    sg_apply_draw_state(&state);
    sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &texvsuni, sizeof(Textured::VSUniform));
    sg_apply_uniform_block(SG_SHADERSTAGE_FS, 0, &texfsuni, sizeof(Textured::FSUniform));
    sg_draw(0, buf->m_count, 1);
}

void DrawSky(const RenderComponent* rc)
{
    const Buffer* buf = Buffers::Get(rc->m_buffer);
    if(!buf)
    {
        return;
    }

    sg_draw_state state     = { 0 };
    state.pipeline          = Pipelines::Get(PT_Sky);
    state.vertex_buffers[0] = buf->m_vertices;
    sg_apply_draw_state(&state);
    sg_apply_uniform_block(SG_SHADERSTAGE_FS, 0, &skyfsuni, sizeof(Sky::FSUniform));
    sg_draw(0, buf->m_count, 1);
}

void Draw()
{
    action.colors[0] = { SG_ACTION_CLEAR, { 0.1f, 0.1f, 0.25f, 1.0f } };
    
    window = Window::GetActive();
    cam = Camera::GetActive();
    
    window->Poll(*cam);
    VP = cam->update();

    sg_begin_default_pass(
        &action, 
        window->m_width,
        window->m_height);
    
    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
    if(ImGui::Begin("Controls"))
    {
        if(ImGui::Button("Sun Dir"))
        {
            texfsuni.LightDir = cam->direction();
        }
        ImGui::SliderFloat("Sun Radiance",      &texfsuni.LightRad, 0.0f, 100.0f);
        ImGui::ColorEdit3("Pal0",               &texfsuni.Pal0.x);
        ImGui::ColorEdit3("Pal1",               &texfsuni.Pal1.x);
        ImGui::ColorEdit3("Pal2",               &texfsuni.Pal2.x);
        ImGui::SliderFloat("PalCenter",         &texfsuni.PalCenter, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",         &texfsuni.RoughnessOffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Metalness",         &texfsuni.MetalnessOffset, -1.0f, 1.0f);
        ImGui::End();
    }

    texfsuni.Eye = cam->m_eye;
    skyfsuni.IVP = glm::inverse(VP);
    skyfsuni.Eye = cam->m_eye;
    skyfsuni.LightDir = texfsuni.LightDir;
    skyfsuni.LightRad = texfsuni.LightRad;

    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        const RenderComponent* rc = Components::Get<RenderComponent>(*s);
        if(!rc)
        {
            continue;
        }

        switch(rc->m_pipeline)
        {
            case PT_Textured:
            {
                DrawTextured(rc);
            }
            break;
            case PT_Sky:
            {
                DrawSky(rc);
            }
            break;
        }
    }

    UIEnd();

    sg_end_pass();
    sg_commit();
    window->Swap();
}
