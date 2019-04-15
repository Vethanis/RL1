#pragma once

#include "slot.h"

struct WindowHandle
{
    void* ptr;
};

namespace Window
{
    WindowHandle Create(const char* title, bool fullscreen);
    void Destroy(WindowHandle window);
    bool IsOpen(WindowHandle window);
    void SetShouldClose(WindowHandle window, bool close);
    void Swap(WindowHandle window);
    void GetSize(WindowHandle window, int32_t& width, int32_t& height);
    void GetCursorPos(WindowHandle window, double& xpos, double& ypos);
    void SetCursorHidden(WindowHandle window, bool hidden);
    WindowHandle GetActive();
    void SetActive(WindowHandle window);
};

struct PushWindow
{
    WindowHandle prev;
    
    inline PushWindow(WindowHandle toPush)
    {
        prev = Window::GetActive();
        Window::SetActive(toPush);
    }
    ~PushWindow()
    {
        Window::SetActive(prev);
    }
};
