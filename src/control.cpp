#include "control.h"

#include "macro.h"
#include "window.h"
#include "sokol_time.h"
#include "circular_stack.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#define GamepadEnabled ((GLFW_VERSION_MAJOR >= 3) && (GLFW_VERSION_MINOR >= 3))

#if GamepadEnabled
    #define GamepadOnly(x) x
#else
    #define GamepadOnly(x)
#endif // Glfw3.3

namespace Ctrl
{
    static constexpr u32 ms_StackCap = 256;

    static CircularStack<u32, ms_StackCap>  ms_channels;
    static CircularStack<f32, ms_StackCap>  ms_values;
    static CircularStack<u64, ms_StackCap>  ms_ticks;

    GamepadOnly(static GLFWgamepadstate ms_prevPad);

    static u64  ms_curTick = 0;
    static f32  ms_scrollX = 0.0f;
    static f32  ms_scrollY = 0.0f;
    static i32  ms_winWidth;
    static i32  ms_winHeight;
    static bool ms_cursorHidden;

    static void PushEvent(u32 ch, f32 value)
    {
        ms_channels.push(ch);
        ms_values.push(value);
        ms_ticks.push(ms_curTick);
    }

    static u64 SecsToTicks(f64 secs)
    {
        return (u64)(secs * 1000000000.0);
    }

    void Init()
    {
        EraseR(ms_ticks);
        EraseR(ms_channels);
        EraseR(ms_values);
    }
    void Update()
    {
        ms_curTick = stm_now();
        glfwPollEvents();

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
                        ms_ticks.push(ms_curTick);
                        ms_channels.push(PadAxis_LX + i);
                        ms_values.push(newAxis);
                    }
                }
                for(u32 i = 0; i < CountOf(state.buttons); ++i)
                {
                    let newBtn = state.buttons[i];
                    let oldBtn = ms_prevPad.axes[i];
                    if(newBtn != oldBtn)
                    {
                        ms_prevPad.buttons[i] = newBtn;
                        ms_ticks.push(ms_curTick);
                        ms_channels.push(PadBtn_A + i);
                        ms_values.push(newBtn ? 1.0f : 0.0f);
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

    void CloseMainWindow()
    {
        Window::SetShouldClose(true);
    }

    bool Get(Channel ch, f64 secsElapsed, Event& evt)
    {
        const u64  maxElapsed = SecsToTicks(secsElapsed);
        const u64  now    = ms_curTick;
        const u64* pTicks = ms_ticks.m_data;
        const u32* pChans = ms_channels.m_data;
        const u32  bot    = ms_channels.bottom_idx();

        for(u32 i  = ms_channels.top_idx();
                i != bot;
                i  = circular_prev<ms_StackCap>(i))
        {
            if((now - pTicks[i]) > maxElapsed)
            {
                return false;
            }
            if(pChans[i] == ch)
            {
                evt.tick = ms_ticks[i];
                evt.value = ms_values[i];
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
        x = x * 2.0f - 1.0f;
        y = y * 2.0f - 1.0f;

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
    void WindowSizeCB(i32 width, i32 height)
    {
        ms_winWidth = width;
        ms_winHeight = height;
    }
};
