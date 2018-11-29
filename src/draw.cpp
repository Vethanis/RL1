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

#include <GLFW/glfw3.h>

sg_pass_action action = {0};

vec3 ldir = glm::normalize(vec3(1.0f));

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

    if(glfwGetKey(window->m_window, GLFW_KEY_E))
    {
        ldir = cam->direction();
    }

    fsuni.Eye           = cam->m_eye;
    fsuni.LightDir      = ldir;
    fsuni.LightRad      = vec3(10.0f);
    fsuni.BumpScale     = 0.05f;
    fsuni.ParallaxScale = 0.05f;

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

    sg_end_pass();
    sg_commit();
    window->Swap();
}
