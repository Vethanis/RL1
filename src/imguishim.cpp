#include "imguishim.h"

#include <GLFW/glfw3.h>
#include "sokol_gfx.h"
#include "sokol_time.h"
#include "imgui.h"

// adapted from https://github.com/floooh/sokol-samples/blob/master/glfw/imgui-glfw.cc

constexpr int32_t MaxVertices = (1<<16);
constexpr int32_t MaxIndices = MaxVertices * 3;

uint64_t last_time = 0;

sg_draw_state draw_state = { };
sg_pass_action pass_action = { };

struct vs_params_t
{
    ImVec2 disp_size;
};

static void draw_cb(ImDrawData* draw_data)
{    
    if (!draw_data || draw_data->CmdListsCount == 0) 
    {
        return;
    }

    // render the command list
    vs_params_t vs_params;
    vs_params.disp_size.x = ImGui::GetIO().DisplaySize.x;
    vs_params.disp_size.y = ImGui::GetIO().DisplaySize.y;
    for (int32_t cl_index = 0; cl_index < draw_data->CmdListsCount; cl_index++) 
    {
        const ImDrawList* cl = draw_data->CmdLists[cl_index];

        // append vertices and indices to buffers, record start offsets in draw state
        const int32_t vtx_size = cl->VtxBuffer.size() * sizeof(ImDrawVert);
        const int32_t idx_size = cl->IdxBuffer.size() * sizeof(ImDrawIdx);
        const int32_t vb_offset = sg_append_buffer(draw_state.vertex_buffers[0], &cl->VtxBuffer.front(), vtx_size);
        const int32_t ib_offset = sg_append_buffer(draw_state.index_buffer, &cl->IdxBuffer.front(), idx_size);
        /* don't render anything if the buffer is in overflow state (this is also
            checked internally in sokol_gfx, draw calls that attempt from
            overflowed buffers will be silently dropped)
        */
        if (sg_query_buffer_overflow(draw_state.vertex_buffers[0]) ||
            sg_query_buffer_overflow(draw_state.index_buffer))
        {
            continue;
        }

        draw_state.vertex_buffer_offsets[0] = vb_offset;
        draw_state.index_buffer_offset = ib_offset;
        sg_apply_draw_state(&draw_state);
        sg_apply_uniform_block(SG_SHADERSTAGE_VS, 0, &vs_params, sizeof(vs_params));

        int32_t base_element = 0;
        for (const ImDrawCmd& pcmd : cl->CmdBuffer) 
        {
            if (pcmd.UserCallback) 
            {
                pcmd.UserCallback(cl, &pcmd);
            }
            else 
            {
                const int32_t scissor_x = (int32_t) (pcmd.ClipRect.x);
                const int32_t scissor_y = (int32_t) (pcmd.ClipRect.y);
                const int32_t scissor_w = (int32_t) (pcmd.ClipRect.z - pcmd.ClipRect.x);
                const int32_t scissor_h = (int32_t) (pcmd.ClipRect.w - pcmd.ClipRect.y);
                sg_apply_scissor_rect(scissor_x, scissor_y, scissor_w, scissor_h, true);
                sg_draw(base_element, pcmd.ElemCount, 1);
            }
            base_element += pcmd.ElemCount;
        }
    }
}

namespace ImGuiShim
{
    void Init(GLFWwindow* window)
    {
        ImGui::CreateContext();
        ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = nullptr;
        io.RenderDrawListsFn = draw_cb;
        io.Fonts->AddFontDefault();
        io.KeyMap[ImGuiKey_Tab] = GLFW_KEY_TAB; 
        io.KeyMap[ImGuiKey_LeftArrow] = GLFW_KEY_LEFT;
        io.KeyMap[ImGuiKey_RightArrow] = GLFW_KEY_RIGHT;
        io.KeyMap[ImGuiKey_UpArrow] = GLFW_KEY_UP;
        io.KeyMap[ImGuiKey_DownArrow] = GLFW_KEY_DOWN;
        io.KeyMap[ImGuiKey_Home] = GLFW_KEY_HOME;
        io.KeyMap[ImGuiKey_End] = GLFW_KEY_END;
        io.KeyMap[ImGuiKey_Delete] = GLFW_KEY_DELETE;
        io.KeyMap[ImGuiKey_Backspace] = GLFW_KEY_BACKSPACE;
        io.KeyMap[ImGuiKey_Enter] = GLFW_KEY_ENTER;
        io.KeyMap[ImGuiKey_Escape] = GLFW_KEY_ESCAPE;
        io.KeyMap[ImGuiKey_A] = GLFW_KEY_A;
        io.KeyMap[ImGuiKey_C] = GLFW_KEY_C;
        io.KeyMap[ImGuiKey_V] = GLFW_KEY_V;
        io.KeyMap[ImGuiKey_X] = GLFW_KEY_X;
        io.KeyMap[ImGuiKey_Y] = GLFW_KEY_Y;
        io.KeyMap[ImGuiKey_Z] = GLFW_KEY_Z;

        sg_buffer_desc vbuf_desc = {};
        vbuf_desc.usage = SG_USAGE_STREAM;
        vbuf_desc.size = MaxVertices * sizeof(ImDrawVert);
        draw_state.vertex_buffers[0] = sg_make_buffer(&vbuf_desc);

        sg_buffer_desc ibuf_desc = { };
        ibuf_desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        ibuf_desc.usage = SG_USAGE_STREAM;
        ibuf_desc.size = MaxIndices * sizeof(ImDrawIdx);
        draw_state.index_buffer = sg_make_buffer(&ibuf_desc);

        uint8_t* font_pixels;
        int32_t font_width, font_height;
        io.Fonts->GetTexDataAsRGBA32(&font_pixels, &font_width, &font_height);
        sg_image_desc img_desc = { };
        img_desc.width = font_width;
        img_desc.height = font_height;
        img_desc.pixel_format = SG_PIXELFORMAT_RGBA8;
        img_desc.wrap_u = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.wrap_v = SG_WRAP_CLAMP_TO_EDGE;
        img_desc.content.subimage[0][0].ptr = font_pixels;
        img_desc.content.subimage[0][0].size = font_width * font_height * 4;
        draw_state.fs_images[0] = sg_make_image(&img_desc);

        sg_shader_desc shd_desc = { };
        auto& ub = shd_desc.vs.uniform_blocks[0];
        ub.size = sizeof(vs_params_t);
        ub.uniforms[0].name = "disp_size";
        ub.uniforms[0].type = SG_UNIFORMTYPE_FLOAT2;
        shd_desc.vs.source =
            "#version 330\n"
            "uniform vec2 disp_size;\n"
            "in vec2 position;\n"
            "in vec2 texcoord0;\n"
            "in vec4 color0;\n"
            "out vec2 uv;\n"
            "out vec4 color;\n"
            "void main() {\n"
            "    gl_Position = vec4(((position/disp_size)-0.5)*vec2(2.0,-2.0), 0.5, 1.0);\n"
            "    uv = texcoord0;\n"
            "    color = color0;\n"
            "}\n";
        shd_desc.fs.images[0].name = "tex";
        shd_desc.fs.images[0].type = SG_IMAGETYPE_2D;
        shd_desc.fs.source =
            "#version 330\n"
            "uniform sampler2D tex;\n"
            "in vec2 uv;\n"
            "in vec4 color;\n"
            "out vec4 frag_color;\n"
            "void main() {\n"
            "    frag_color = texture(tex, uv) * color;\n"
            "}\n";
        sg_shader shd = sg_make_shader(&shd_desc);

        // pipeline object for imgui rendering
        sg_pipeline_desc pip_desc = { };
        pip_desc.layout.buffers[0].stride = sizeof(ImDrawVert);
        auto& attrs = pip_desc.layout.attrs;
        attrs[0].name = "position";  attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
        attrs[1].name = "texcoord0"; attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
        attrs[2].name = "color0";    attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
        pip_desc.shader = shd;
        pip_desc.index_type = SG_INDEXTYPE_UINT16;
        pip_desc.blend.enabled = true;
        pip_desc.blend.src_factor_rgb = SG_BLENDFACTOR_SRC_ALPHA;
        pip_desc.blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
        pip_desc.blend.color_write_mask = SG_COLORMASK_RGB;
        draw_state.pipeline = sg_make_pipeline(&pip_desc);

        // initial clear color
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        pass_action.colors[0].val[0] = 0.0f;
        pass_action.colors[0].val[1] = 0.5f;
        pass_action.colors[0].val[2] = 0.7f;
        pass_action.colors[0].val[3] = 1.0f;


    }
    void Begin(GLFWwindow* window)
    {        
        int32_t cur_width, cur_height;
        glfwGetFramebufferSize(window, &cur_width, &cur_height);

        ImGuiIO& io = ImGui::GetIO();
        io.DisplaySize = ImVec2(float(cur_width), float(cur_height));
        io.DeltaTime = (float)stm_sec(stm_laptime(&last_time));
        ImGui::NewFrame();
    }
    void End()
    {
        ImGui::Render();
    }
    void Shutdown()
    {
        ImGui::DestroyContext();
    }
};
