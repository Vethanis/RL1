#include "draw.h"

#include "macro.h"
#include "component.h"
#include "transform.h"
#include "buffer.h"
#include "image.h"
#include "pipeline.h"
#include "window.h"
#include "camera.h"

#include "sokol_gfx.h"

sg_pass_action action = {0};

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
    
    for(const slot* s = Components::begin(); s != Components::end(); ++s)
    {
        const Row& row = Components::GetUnchecked(*s);
        const RenderComponent* rc = row.Get<RenderComponent>();
        const TransformComponent* xform = row.Get<TransformComponent>();
        if(!rc || !xform)
        {
            continue;
        }

        const Buffer* buf = Buffers::Get(rc->m_buf);
        const sg_image* img = Images::Get(rc->m_img);
        const sg_pipeline* pipe = Pipelines::Get(rc->m_pipeline);

        if(!buf || !img || !pipe)
        {
            continue;
        }

        sg_draw_state state = {0};
        state.vertex_buffers[0] = buf->m_id;
        state.fs_images[0] = *img;
        state.pipeline = *pipe;

        Transform MVP = VP * xform->m_matrix;

        sg_apply_draw_state(&state);
        sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &MVP, sizeof(Transform));
        sg_draw(0, buf->m_count, 1);
    }

    sg_end_pass();
    sg_commit();
    window->Swap();
}
