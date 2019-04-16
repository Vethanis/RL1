#pragma once

struct GLFWwindow;

namespace Window
{
    void Init();
    void Update();
    void Shutdown();

    bool IsOpen();
    void SetShouldClose(bool close);
    void SetCursorHidden(bool hidden);

    GLFWwindow* Get();
};
