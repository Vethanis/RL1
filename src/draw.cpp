#include "draw.h"

#include "macro.h"
#include "component.h"
#include "transform.h"
#include "buffer.h"
#include "image.h"
#include "pipeline.h"
#include "window.h"
#include "camera.h"
#include "prng.h"

#include "shaders/textured.h"

#include "sokol_gfx.h"
#include "uidraw.h"
#include "imgui.h"


sg_pass_action action = {0};

vec3 lcol = vec3(1.0f);
float lrad = 10.0f;

VSUniform vsuni;
FSUniform fsuni = 
{
    vec3(0.0f),
    glm::normalize(vec3(1.0f)),
    vec3(10.0f),
    0.085f,
    0.026f,
    0.0f,
    0.0f,
    1.0f
};

void Draw()
{
    action.colors[0] = { SG_ACTION_CLEAR, { 0.1f, 0.1f, 0.25f, 1.0f } };
    
    Window* window = Window::GetActive();
    Camera* cam = Camera::GetActive();
    
    window->Poll(*cam);
    Transform VP = cam->update();

    sg_begin_default_pass(
        &action, 
        window->m_width,
        window->m_height);
    
    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
    if(ImGui::Begin("Controls"))
    {
        if(ImGui::Button("Sun Dir"))
        {
            fsuni.LightDir = cam->direction();
        }
        ImGui::SliderFloat3("Sun Color",        &lcol.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Sun Radiance",      &lrad, 0.0f, 1000.0f);
        ImGui::SliderFloat("Roughness",         &fsuni.RoughnessOffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Metalness",         &fsuni.MetalnessOffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Bump Scale",        &fsuni.BumpScale, 0.0f, 0.25f);
        ImGui::SliderFloat("Parallax Scale",    &fsuni.ParallaxScale, 0.0f, 0.25f);
        ImGui::End();
    }

    fsuni.Eye           = cam->m_eye;
    fsuni.LightRad      = lrad * lcol;

    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        const Row&                  row     = Components::GetUnchecked(*s);
        const RenderComponent*      rc      = row.Get<RenderComponent>();
        const TransformComponent*   tc      = row.Get<TransformComponent>();
        
        if(!rc || !tc)
        {
            continue;
        }

        const Buffer*       buf         = Buffers::Get(rc->m_buffer);
        const sg_image*     material    = Images::Get(rc->m_material);
        const sg_image*     palette     = Images::Get(rc->m_palette);
        const sg_pipeline*  pipe        = Pipelines::Get(rc->m_pipeline);

        if(!buf || !material || !palette || !pipe)
        {
            continue;
        }

        vsuni.MVP   = VP * tc->m_matrix;
        vsuni.M     = tc->m_matrix;
        fsuni.Seed  = Randf();

        sg_draw_state state     = { 0 };
        state.pipeline          = *pipe;
        state.vertex_buffers[0] = buf->m_vertices;
        state.index_buffer      = buf->m_indices;
        state.fs_images[0]      = *material;
        state.fs_images[1]      = *palette;

        sg_apply_draw_state(&state);
        sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &vsuni, sizeof(VSUniform));
        sg_apply_uniform_block(SG_SHADERSTAGE_FS, 0, &fsuni, sizeof(FSUniform));
        sg_draw(0, buf->m_count, 1);
    }

    UIEnd();

    sg_end_pass();
    sg_commit();
    window->Swap();
}
