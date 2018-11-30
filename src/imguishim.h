#pragma once

struct GLFWwindow;

namespace ImGuiShim
{
    void Init(GLFWwindow* window);
    void Begin(GLFWwindow* window);
    void End();
    void Shutdown();
};
