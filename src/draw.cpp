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
#include "vkrenderer.h"
#include "imgui.h"

static mat4                VP;
static Textured::VSUniform vsuni;
static Textured::FSUniform fsuni = 
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
static Flat::VSUniform flatvsuni;
static Flat::FSUniform flatfsuni;
static float debugOrthoScale = 5.0f;
static int32_t frameNo = 0;
static Camera bgCam;

static const vec4 debugViewports[] = 
{
    vec4(0.0f, 0.5f, 0.5f, 0.5f),   // TL
    vec4(0.5f, 0.5f, 0.5f, 0.5f),   // TR
    vec4(0.0f, 0.0f, 0.5f, 0.5f),   // BL
    vec4(0.5f, 0.0f, 0.5f, 0.5f)    // BR
};
static const vec3 debugDirs[] = 
{
    vec3(0.0f, 1.0f, 0.0f),     // up
    vec3(1.0f),                 // iso
    vec3(0.0f, 0.0f, 1.0f),     // fwd
    vec3(1.0f, 0.0f, 0.0f)      // right
};
static const vec3 debugUps[] = 
{
    vec3(0.0f, 0.0f, -1.0f),    // fwd
    vec3(0.0f, 1.0f, 0.0f),     // up
    vec3(0.0f, 1.0f, 0.0f),     // up
    vec3(0.0f, 1.0f, 0.0f)      // up
};

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

    GL_ONLY(Renderer::DrawTextured(
        *buf, 
        *material, 
        *normal, 
        vsuni, 
        fsuni));
}

inline void DrawFlat(const RenderComponent* rc)
{
    if(const Renderer::Buffer* buf = Buffers::Get(rc->m_buffer))
    {
        flatvsuni.MVP   = VP * rc->m_matrix;
        flatvsuni.M     = rc->m_matrix;
        flatfsuni.Seed  = Randf();

        GL_ONLY(Renderer::DrawFlat(
            *buf,
            flatvsuni, 
            flatfsuni));
    }
}

void FirstDraw()
{

}

void Draw()
{
    if(frameNo == 0)
    {
        FirstDraw();
    }
    ++frameNo;

    Camera* cam = Camera::GetActive();
    VP = cam->update();

    ImGui::SetNextWindowSize(ImVec2(800.0f, 600.0f));
    if(ImGui::Begin("Controls"))
    {
        if(ImGui::Button("Sun Dir"))
        {
            fsuni.LightDir = cam->direction();
        }
        ImGui::SliderFloat("Ortho Scale",       &debugOrthoScale, 0.1f, 50.0f);
        ImGui::SliderFloat("Sun Radiance",      &fsuni.LightRad, 0.0f, 25.0f);
        ImGui::ColorEdit3("Pal0",               &fsuni.Pal0.x);
        ImGui::ColorEdit3("Pal1",               &fsuni.Pal1.x);
        ImGui::ColorEdit3("Pal2",               &fsuni.Pal2.x);
        ImGui::SliderFloat("PalCenter",         &fsuni.PalCenter, 0.0f, 1.0f);
        ImGui::SliderFloat("Roughness",         &fsuni.RoughnessOffset, -1.0f, 1.0f);
        ImGui::SliderFloat("Metalness",         &fsuni.MetalnessOffset, -1.0f, 1.0f);
        ImGui::End();
    }

    fsuni.Eye = cam->m_eye;
    flatfsuni.Albedo = fsuni.Pal0;
    flatfsuni.Eye = fsuni.Eye;
    flatfsuni.LightDir = fsuni.LightDir;
    flatfsuni.LightRad = fsuni.LightRad;
    flatfsuni.Metalness = fsuni.MetalnessOffset;
    flatfsuni.Roughness = fsuni.RoughnessOffset;

    GL_ONLY(Renderer::Begin());
    VK_ONLY(VkRenderer::Begin());
    if(true)
    {
        for(const slot* s = Components::begin(); s != Components::end(); ++s)
        {
            if(const RenderComponent* rc = Components::Get<RenderComponent>(*s))
            {
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
        }
        Renderer::DrawBackground();
    }
    else 
    {
        mat4 perspective = glm::ortho(-debugOrthoScale, debugOrthoScale, -debugOrthoScale, debugOrthoScale, -10.0f, 10.0f);
        bgCam.P = perspective;
        for(int32_t i = 0; i < 4; ++i)
        {
            GL_ONLY(Renderer::SetViewport(debugViewports[i]));
            VK_ONLY(VkRenderer::SetViewport(debugViewports[i]));
            mat4 view = glm::lookAt(cam->m_eye, cam->m_eye - debugOrthoScale * debugDirs[i], debugUps[i]);
            VP = perspective * view;
            bgCam.V = view;
            for(const slot* s = Components::begin(); s != Components::end(); ++s)
            {
                if(const RenderComponent* rc = Components::Get<RenderComponent>(*s))
                {
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
            }
            PushCamera pushCam(&bgCam);
            Renderer::DrawBackground();
        }
    }

    Renderer::End();
}
