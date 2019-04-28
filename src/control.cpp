#include "control.h"

#include "lang.h"
#include "window.h"
#include "sokol_time.h"
#include "circular_stack.h"
#include "imvis.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GamepadEnabled ((GLFW_VERSION_MAJOR >= 3) && (GLFW_VERSION_MINOR >= 3))

#if GamepadEnabled
    #define GamepadOnly(x) x
#else
    #define GamepadOnly(x)
#endif // Glfw3.3

static constexpr u32 ms_StackCap = 256;

static CircularStack<u32, ms_StackCap>  ms_channels;
static CircularStack<f32, ms_StackCap>  ms_values;
static CircularStack<u64, ms_StackCap>  ms_ticks;

GamepadOnly(static GLFWgamepadstate ms_prevPad);

static u64  ms_now;
static i32  ms_winWidth;
static i32  ms_winHeight;
static f32  ms_cursor[2];
static f32  ms_prevCursor[2];
static bool ms_cursorHidden;

static i32  FindAfter(Channel ch, u64 after);
static i32  FindBefore(Channel ch, u64 before);

static void PushEvent(u32 ch, f32 value)
{
    ms_channels.push(ch);
    ms_values.push(value);
    ms_ticks.push(ms_now);
}
static u32 FirstEventIdx()
{
    return ms_channels.peek_idx();
}
static u32 NextEventIdx(u32 idx)
{
    return circular_prev<ms_StackCap>(idx);
}

namespace Ctrl
{
    void Init()
    {
        EraseR(ms_ticks);
        EraseR(ms_channels);
        EraseR(ms_values);
    }
    void Update()
    {
        ms_now = stm_now();
        glfwPollEvents();

        if(ms_prevCursor[0] != ms_cursor[0])
        {
            ms_prevCursor[0] = ms_cursor[0];
            PushEvent(MouseAxis_X, ms_cursor[0]);
        }
        if(ms_prevCursor[1] != ms_cursor[1])
        {
            ms_prevCursor[1] = ms_cursor[1];
            PushEvent(MouseAxis_Y, ms_cursor[1]);
        }

        #if GamepadEnabled
        {
            GLFWgamepadstate state;
            EraseR(state);
            if(glfwGetGamepadState(0, &state) == GLFW_TRUE)
            {
                for(u32 i = 0; i < CountOf(state.axes); ++i)
                {
                    let newAxis = state.axes[i];
                    let oldAxis = ms_prevPad.axes[i];
                    if(newAxis != oldAxis)
                    {
                        ms_prevPad.axes[i] = newAxis;
                        PushEvent(PadAxis_LX + i, newAxis);
                    }
                }
                for(u32 i = 0; i < CountOf(state.buttons); ++i)
                {
                    let newBtn = state.buttons[i];
                    let oldBtn = ms_prevPad.axes[i];
                    if(newBtn != GLFW_REPEAT)
                    {
                        if(newBtn != oldBtn)
                        {
                            ms_prevPad.buttons[i] = newBtn;
                            PushEvent(PadBtn_A + i, newBtn ? 1.0f : 0.0f);
                        }
                    }
                }
            }
        }
        #endif // GamepadEnabled

    }
    void Shutdown()
    {
        EraseR(ms_ticks);
        EraseR(ms_channels);
        EraseR(ms_values);
    }

    void SetCursorHidden(bool hidden)
    {
        ms_cursorHidden = hidden;
        Window::SetCursorHidden(hidden);
    }
    bool IsCursorHidden()
    {
        return ms_cursorHidden;
    }

    bool GetAfter(Channel ch, u64 after, Event* out)
    {
        let i = FindAfter(ch, after);
        if(i == -1)
        {
            return false;
        }
        if(out)
        {
            out->channel    = ch;
            out->tick       = ms_ticks[i];
            out->value      = ms_values[i];
        }
        return true;
    }
    bool GetBefore(Channel ch, u64 before, Event* out)
    {
        let i = FindBefore(ch, before);
        if(i == -1)
        {
            return false;
        }
        if(out)
        {
            out->channel    = ch;
            out->tick       = ms_ticks[i];
            out->value      = ms_values[i];
        }
        return true;
    }

    void KeyCB(i32 key, i32 action, i32 mods)
    {
        PushEvent(key, action ? 1.0f : 0.0f);
    }
    void MouseButtonCB(i32 button, i32 action, i32 mods)
    {
        PushEvent(MouseBtn_1 + button, action ? 1.0f : 0.0f);
    }
    void CursorPosCB(f32 x, f32 y)
    {
        x /= (f32)ms_winWidth;
        y /= (f32)ms_winHeight;
        x = x *  2.0f - 1.0f;
        y = y * -2.0f + 1.0f;

        ms_cursor[0] = x;
        ms_cursor[1] = y;
    }
    void ScrollWheelCB(f32, f32 z)
    {
        PushEvent(MouseAxis_Z, z);
    }
    void WindowSizeCB(i32 width, i32 height)
    {
        ms_winWidth = width;
        ms_winHeight = height;
        PushEvent(Window_Width, (f32)width);
        PushEvent(Window_Height, (f32)height);
    }

    static bool ms_imvisEnabled = false;
    bool& ImVisEnabled()
    {
        return ms_imvisEnabled;
    }
    void ImVisUpdate()
    {
        ImGui::SetNextWindowSize({ 400, 400 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Ctrl");

        let channels = ms_channels.m_data;
        let ticks = ms_ticks.m_data;
        let values = ms_values.m_data;

        ImGui::Columns(3);
        ImGui::Text("%-12s", "Channel");    ImGui::NextColumn();
        ImGui::Text("%-12s", "Value");      ImGui::NextColumn();
        ImGui::Text("%-12s", "Seconds");    ImGui::NextColumn();
        ImGui::Separator();

        let start = FirstEventIdx();
        u32 i = start;
        do
        {
            let channel = GetChannelStr((Channel)channels[i]);
            let value = values[i];
            let secs = stm_sec(ticks[i]);

            ImGui::Text("%-12s", channel);  ImGui::NextColumn();
            ImGui::Text("%-12g", value);    ImGui::NextColumn();
            ImGui::Text("%-12g", secs);     ImGui::NextColumn();
            ImGui::Separator();

            i = NextEventIdx(i);
        } while(i != start);

        ImGui::Columns();

        ImGui::End();
    }
};

static i32 FindAfter(Channel ch, u64 after)
{
    let chans = ms_channels.m_data;
    let ticks = ms_ticks.m_data;
    let start = FirstEventIdx();
    u32 i = start;
    do
    {
        if(ticks[i] <= after)
        {
            return -1;
        }
        if(chans[i] == ch)
        {
            return i;
        }
        i = NextEventIdx(i);
    } while(i != start);
    return -1;
}

static i32 FindBefore(Channel ch, u64 before)
{
    let chans = ms_channels.m_data;
    let ticks = ms_ticks.m_data;
    let start = FirstEventIdx();
    u32 i = start;
    do
    {
        if(ticks[i] < before)
        {
            if(chans[i] == ch)
            {
                return i;
            }
        }
        i = NextEventIdx(i);
    } while(i != start);
    return -1;
}
