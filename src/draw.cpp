#include "draw.h"

#include "macro.h"
#include "component.h"
#include "rendercomponent.h"
#include "buffer.h"
#include "image.h"
#include "camera.h"
#include "window.h"
#include "prng.h"
#include "renderer.h"
#include "imgui.h"

mat4                VP;
Textured::VSUniform vsuni;
Textured::FSUniform fsuni = 
{
    vec3(0.0f),
    glm::normalize(vec3(1.0f)),
    vec3(0.0f),
    vec3(1.0f, 0.0f, 0.0f),
    vec3(1.0f),
    1.0f,
    1.0f,
    0.2f,
    0.0f
};
Flat::VSUniform flatvsuni;
Flat::FSUniform flatfsuni;

inline void DrawTextured(const RenderComponent* rc)
{
    const Renderer::Buffer*     buf         = Buffers::Get(rc->m_buffer);
    const Renderer::Texture*    material    = Images::Get(rc->m_material);
    const Renderer::Texture*    normal      = Images::Get(rc->m_normal);

    if(!buf || !material || !normal)
    {
        return;
    }

    vsuni.MVP   = VP * rc->m_matrix;
    vsuni.M     = rc->m_matrix;
    fsuni.Seed  = Randf();

    Renderer::DrawTextured(
        *buf, 
        *material, 
        *normal, 
        vsuni, 
        fsuni);
}

inline void DrawFlat(const RenderComponent* rc)
{
    const Renderer::Buffer*     buf         = Buffers::Get(rc->m_buffer);
    if(!buf)
    {
        return;
    }

    flatvsuni.MVP   = VP * rc->m_matrix;
    flatvsuni.M     = rc->m_matrix;
    flatfsuni.Seed  = Randf();

    Renderer::DrawFlat(
        *buf,
        flatvsuni, 
        flatfsuni);
}

void Draw()
{
    Camera* cam = Camera::GetActive();
    Window::GetActive()->Poll(*cam);
    VP = cam->update();
    fsuni.Eye = cam->m_eye;

    Renderer::Begin();
    
    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
    if(ImGui::Begin("Controls"))
    {
        if(ImGui::Button("Sun Dir"))
        {
            fsuni.LightDir = cam->direction();
        }
        ImGui::SliderFloat("Sun Radiance",      &fsuni.LightRad, 0.0f, 25.0f);
        ImGui::ColorEdit3("Pal0",               &fsuni.Pal0.x);
        ImGui::ColorEdit3("Pal1",               &fsuni.Pal1.x);
        ImGui::ColorEdit3("Pal2",               &fsuni.Pal2.x);
        ImGui::SliderFloat("PalCenter",         &fsuni.PalCenter, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",         &fsuni.RoughnessOffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Metalness",         &fsuni.MetalnessOffset, -1.0f, 1.0f);
        ImGui::End();
    }

    flatfsuni.Albedo = fsuni.Pal0;
    flatfsuni.Eye = fsuni.Eye;
    flatfsuni.LightDir = fsuni.LightDir;
    flatfsuni.LightRad = fsuni.LightRad;
    flatfsuni.Metalness = fsuni.MetalnessOffset;
    flatfsuni.Roughness = fsuni.RoughnessOffset;

    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        const RenderComponent* rc = Components::Get<RenderComponent>(*s);
        if(!rc)
        {
            continue;
        }
        switch(rc->m_type)
        {
            case PT_Textured:
                DrawTextured(rc);
            break;
            case PT_Flat:
                DrawFlat(rc);
            break;
        }
    }

    Renderer::End();
}
