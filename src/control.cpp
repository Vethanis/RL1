#include "control.h"

#include "lang.h"
#include "window.h"
#include "sokol_time.h"
#include "circular_stack.h"
#include "imvis.h"
#include "hlsl_types.h"
#include "math.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

def u32 StackCapacity = 256;

using Channels  = CircularStack<u32, StackCapacity>;
using Values    = CircularStack<f32, StackCapacity>;
using Ticks     = CircularStack<u64, StackCapacity>;

static u64      ms_now;
static Channels ms_channels;
static Values   ms_values;
static Ticks    ms_ticks;
static float2   ms_cursor;
static float2   ms_prevCursor;
static int2     ms_windowSize;
static bool     ms_cursorHidden;
static i32      ms_stackVisLen = 20;

#define NextEventIdx(x) circular_prev<StackCapacity>(x)
#define FirstEventIdx() (ms_channels.peek_idx())

static i32  FindAfter(Channel ch, u64 after);
static i32  FindBefore(Channel ch, u64 before);
static void PushEvent(u32 ch, f32 value);

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

        for(u32 i = 0; i < 2; ++i)
        {
            if(ms_cursor[i] != ms_prevCursor[i])
            {
                ms_prevCursor[i] = ms_cursor[i];
                PushEvent(MouseAxis_X + i, ms_cursor[i]);
            }
        }
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
        x /= (f32)ms_windowSize.x;
        y /= (f32)ms_windowSize.y;
        x = x *  2.0f - 1.0f;
        y = y * -2.0f + 1.0f;

        ms_cursor.x = x;
        ms_cursor.y = y;
    }
    void ScrollWheelCB(f32, f32 z)
    {
        PushEvent(MouseAxis_Z, z);
    }
    void WindowSizeCB(i32 width, i32 height)
    {
        ms_windowSize = { width, height };
        PushEvent(Window_Width, (f32)width);
        PushEvent(Window_Height, (f32)height);
    }

    void ImVisUpdate()
    {
        ImGui::SetNextWindowSize({ 300, 500 }, ImGuiCond_FirstUseEver);
        ImGui::Begin("Ctrl");

        let channels = ms_channels.m_data;
        let ticks = ms_ticks.m_data;
        let values = ms_values.m_data;

        ImGui::SliderInt("Length", &ms_stackVisLen, 0, StackCapacity);

        ImGui::Columns(3);
        ImGui::Text("%-12s", "Channel");    ImGui::NextColumn();
        ImGui::Text("%-5s",  "Value");      ImGui::NextColumn();
        ImGui::Text("%-5s",  "Time");       ImGui::NextColumn();
        ImGui::Separator();

        for(u32 i = FirstEventIdx(), j = 0; j < ms_stackVisLen; ++j)
        {
            let channel = GetChannelStr((Channel)channels[i]);
            let value = values[i];
            let secs = stm_sec(ticks[i]);

            ImGui::Text("%-12s", channel);  ImGui::NextColumn();
            ImGui::Text("%05.2f", value);   ImGui::NextColumn();
            ImGui::Text("%05.2f", secs);    ImGui::NextColumn();
            ImGui::Separator();

            i = NextEventIdx(i);
        }

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

static void PushEvent(u32 ch, f32 value)
{
    ms_channels.push(ch);
    ms_values.push(value);
    ms_ticks.push(ms_now);
}
