#pragma once

#include <stdint.h>

struct GLFWwindow;

namespace Window
{
    GLFWwindow* Init(const char* title, bool fullscreen);
    void Shutdown(GLFWwindow* window);
    bool IsOpen(GLFWwindow* window);
    void SetShouldClose(GLFWwindow* window, bool close);
    void Swap(GLFWwindow* window);
    void GetSize(GLFWwindow* window, int32_t& width, int32_t& height);
    void GetCursorPos(GLFWwindow* window, double& xpos, double& ypos);
    void SetCursorHidden(GLFWwindow* window, bool hidden);
    GLFWwindow* GetActive();
    void SetActive(GLFWwindow* window);
};

struct PushWindow
{
    GLFWwindow* prev;
    
    inline PushWindow(GLFWwindow* toPush)
    {
        prev = Window::GetActive();
        Window::SetActive(toPush);
    }
    ~PushWindow()
    {
        Window::SetActive(prev);
    }
};
