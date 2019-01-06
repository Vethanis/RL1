#pragma once

#include <stdint.h>

struct GLFWwindow;

struct Window
{
    GLFWwindow* m_window;
    int32_t     m_width;
    int32_t     m_height;
    float       m_dt;
    float       m_cx;
    float       m_cy;
    float       m_dcx;
    float       m_dcy;

    void Init(const char* title, bool fullscreen);
    void Shutdown();
    bool Open();
    void Swap();

    static Window* GetActive();
    static void SetActive(Window* window);
};
