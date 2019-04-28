#pragma once

#include "lang.h"
#include "channel.h"

namespace Ctrl
{
    struct Event
    {
        Channel channel;
        f32     value;
        u64     tick;
    };

    void Init();
    void Update();
    void Shutdown();

    void SetCursorHidden(bool hidden);
    bool IsCursorHidden();

    // get event on channel after tick occurred (newer event)
    bool GetAfter(Channel ch, u64 after, Event* out = nullptr);

    // get event on channel before tick occurred (older event)
    bool GetBefore(Channel ch, u64 before, Event* out = nullptr);

    void KeyCB(i32 key, i32 action, i32 mods);
    void MouseButtonCB(i32 button, i32 action, i32 mods);
    void CursorPosCB(f32 x, f32 y);
    void ScrollWheelCB(f32 dx, f32 dy);
    void WindowSizeCB(i32 width, i32 height);

    bool& ImVisEnabled();
    void ImVisUpdate();
};
