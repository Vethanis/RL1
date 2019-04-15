#include "control.h"

#include "macro.h"
#include "window.h"
#include "sokol_time.h"
#include "circular_stack.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Ctrl
{
    static constexpr u32 ms_StackCap = 256;

    static CircularStack<u32, ms_StackCap>  ms_channels;
    static CircularStack<f32, ms_StackCap>  ms_values;
    static CircularStack<u64, ms_StackCap>  ms_ticks;

    static GLFWgamepadstate ms_prevPad;

    static f32  ms_scrollX = 0.0f;
    static f32  ms_scrollY = 0.0f;
    static i32  ms_winWidth;
    static i32  ms_winHeight;

    static void PushEvent(u32 ch, f32 value)
    {
        ms_channels.push(ch);
        ms_values.push(value);
        ms_ticks.push(stm_now());
    }

    void Init()
    {
        EraseR(ms_ticks);
        EraseR(ms_channels);
        EraseR(ms_values);
    }
    void Update(f32 dt)
    {
        Window::GetSize(Window::GetActive(), ms_winWidth, ms_winHeight);
        glfwPollEvents();
        {
            GLFWgamepadstate state;
            EraseR(state);
            if(glfwGetGamepadState(0, &state) == GLFW_TRUE)
            {
                const u64 tick = stm_now();
                for(u32 i = 0; i < CountOf(state.axes); ++i)
                {
                    if(state.axes[i] != ms_prevPad.axes[i])
                    {
                        ms_prevPad.axes[i] = state.axes[i];
                        ms_ticks.push(tick);
                        ms_channels.push(PadAxis_LX + i);
                        ms_values.push(state.axes[i]);
                    }
                }
                for(u32 i = 0; i < CountOf(state.buttons); ++i)
                {
                    if(state.buttons[i] != ms_prevPad.axes[i])
                    {
                        ms_prevPad.buttons[i] = state.buttons[i];
                        ms_ticks.push(tick);
                        ms_channels.push(PadBtn_A + i);
                        ms_values.push(state.buttons ? 1.0f : 0.0f);
                    }
                }
            }
        }
    }
    void Shutdown()
    {
        EraseR(ms_ticks);
        EraseR(ms_channels);
        EraseR(ms_values);
    }

    static bool ms_cursorHidden;
    void SetCursorHidden(bool hidden)
    {
        ms_cursorHidden = hidden;
        Window::SetCursorHidden(Window::GetActive(), hidden);
    }
    bool IsCursorHidden()
    {
        return ms_cursorHidden;
    }

    void CloseMainWindow()
    {
        Window::SetShouldClose(Window::GetActive(), true);
    }

    bool Get(Channel ch, f64 secsElapsed, Event& evtOut)
    {
        const u64 maxElapsed = (u64)(secsElapsed * 1000000000.0);
        const u64 now        = stm_now();

        const u64* pTicks   = ms_ticks.m_data;
        const u32* pChans   = ms_channels.m_data;
        const u32 bot       = ms_channels.bottom_idx();

        for(u32 i  = ms_channels.top_idx();
                i != bot;
                i  = circular_prev<ms_StackCap>(i))
        {
            const u64 elapsed = now - pTicks[i];
            if(elapsed > maxElapsed)
            {
                return false;
            }
            if(pChans[i] == ch)
            {
                evtOut.channel = ch;
                evtOut.tick    = ms_ticks[i];
                evtOut.value   = ms_values[i];
                return true;
            }
        }

        return false;
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
        x = x * 2.0 - 1.0;
        y = y * 2.0 - 1.0;

        PushEvent(MouseAxis_X, -x);
        PushEvent(MouseAxis_Y, -y);
    }
    void ScrollWheelCB(f32 dx, f32 dy)
    {
        ms_scrollX += dx;
        ms_scrollY += dy;

        PushEvent(MouseAxis_Z, ms_scrollX);
        PushEvent(MouseAxis_W, ms_scrollY);
    }
};
