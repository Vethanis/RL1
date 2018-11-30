#include "draw.h"

#include "macro.h"
#include "component.h"
#include "transform.h"
#include "buffer.h"
#include "image.h"
#include "pipeline.h"
#include "window.h"
#include "camera.h"

#include "shaders/textured.h"

#include "sokol_gfx.h"
#include "uidraw.h"
#include "imgui.h"

sg_pass_action action = {0};

vec3 ldir = glm::normalize(vec3(1.0f));
vec3 lcol = vec3(1.0f);
float lrad = 10.0f;
float roughoffset = 0.0f;
float metaloffset = 0.0f;
float bumpScale = 0.066f;
float paraScale = 0.026f;

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
    
    VSUniform vsuni;
    FSUniform fsuni;

    if(ImGui::Begin("Controls"))
    {
        if(ImGui::Button("Sun Dir"))
        {
            ldir = cam->direction();
        }
        ImGui::SliderFloat3("Sun Color", &lcol.x, 0.0f, 1.0f);
        ImGui::SliderFloat("Sun Radiance", &lrad, 0.0f, 1000.0f);
        ImGui::SliderFloat("Roughness", &roughoffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Metalness", &metaloffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Bump Scale", &bumpScale, 0.0f, 0.25f);
        ImGui::SliderFloat("Parallax Scale", &paraScale, 0.0f, 0.25f);
        ImGui::End();
    }

    fsuni.Eye           = cam->m_eye;
    fsuni.LightDir      = ldir;
    fsuni.LightRad      = lrad * lcol;
    fsuni.BumpScale     = bumpScale;
    fsuni.ParallaxScale = paraScale;
    fsuni.RoughnessOffset = roughoffset;
    fsuni.MetalnessOffset = metaloffset;

    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        const Row&                  row     = Components::GetUnchecked(*s);
        const RenderComponent*      rc      = row.Get<RenderComponent>();
        const TransformComponent*   xform   = row.Get<TransformComponent>();
        
        if(!rc || !xform)
        {
            continue;
        }

        const Buffer*       buf         = Buffers::Get(rc->m_buf);
        const sg_image*     material    = Images::Get(rc->m_material);
        const sg_image*     palette     = Images::Get(rc->m_palette);
        const sg_pipeline*  pipe        = Pipelines::Get(rc->m_pipeline);

        if(!buf || !palette || !material || !pipe)
        {
            continue;
        }

        sg_draw_state state     = {0};
        state.vertex_buffers[0] = buf->m_id;
        state.fs_images[0]      = *material;
        state.fs_images[1]      = *palette;
        state.pipeline          = *pipe;

        vsuni.MVP = VP * xform->m_matrix;
        vsuni.M = xform->m_matrix;

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
