#include "window.h"

#include <stdio.h>
#include "macro.h"
#include "control.h"
#include "imgui.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#if _DEBUG
    static void APIENTRY glDebugOutput(
        GLenum          source,
        GLenum          type,
        GLuint          id,
        GLenum          severity,
        GLsizei         length,
        const GLchar*   message,
        const void*     userParam)
    {
        if(id == 131169 || id == 131185 || id == 131218 || id == 131204)
        {
            return;
        } 

        puts("-----------------------------");
        printf("Debug message (%d): %s\n", id, message);
        switch (source)
        {
            case GL_DEBUG_SOURCE_API:             puts("Source: API"); break;
            case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   puts("Source: Window System"); break;
            case GL_DEBUG_SOURCE_SHADER_COMPILER: puts("Source: Shader Compiler"); break;
            case GL_DEBUG_SOURCE_THIRD_PARTY:     puts("Source: Third Party"); break;
            case GL_DEBUG_SOURCE_APPLICATION:     puts("Source: Application"); break;
            case GL_DEBUG_SOURCE_OTHER:           puts("Source: Other"); break;
        }

        switch (type)
        {
            case GL_DEBUG_TYPE_ERROR:               puts("Type: Error"); break;
            case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: puts("Type: Deprecated Behaviour"); break;
            case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  puts("Type: Undefined Behaviour"); break; 
            case GL_DEBUG_TYPE_PORTABILITY:         puts("Type: Portability"); break;
            case GL_DEBUG_TYPE_PERFORMANCE:         puts("Type: Performance"); break;
            case GL_DEBUG_TYPE_MARKER:              puts("Type: Marker"); break;
            case GL_DEBUG_TYPE_PUSH_GROUP:          puts("Type: Push Group"); break;
            case GL_DEBUG_TYPE_POP_GROUP:           puts("Type: Pop Group"); break;
            case GL_DEBUG_TYPE_OTHER:               puts("Type: Other"); break;
        }
        
        switch (severity)
        {
            case GL_DEBUG_SEVERITY_HIGH:         puts("Severity: high"); break;
            case GL_DEBUG_SEVERITY_MEDIUM:       puts("Severity: medium"); break;
            case GL_DEBUG_SEVERITY_LOW:          puts("Severity: low"); break;
            case GL_DEBUG_SEVERITY_NOTIFICATION: puts("Severity: notification"); break;
        }
        puts("");

        DebugAssert(false);
    }
#endif // _DEBUG

namespace Window
{
    static WindowHandle ms_active;
    static i32          ms_glfwRefs;
    static bool         ms_gladInit;

    static GLFWwindow* ToWindow(WindowHandle hdl)
    {
        DebugAssert(hdl.ptr);
        return (GLFWwindow*)hdl.ptr;
    }

    WindowHandle Create(const char* title, bool fullscreen)
    {
        ++ms_glfwRefs;
        if(ms_glfwRefs == 1)
        {
            i32 glfwLoaded = glfwInit();
            DebugAssert(glfwLoaded);
        }

        glfwSetErrorCallback(
            [](i32 error, const char* msg)
            {
                fprintf(stdout, "%d :: %s\n", error, msg);
            });

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        DebugAssert(monitor);

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        DebugAssert(mode);
        
        glfwWindowHint(GLFW_RED_BITS,       mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS,     mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,      mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE,   mode->refreshRate);
        glfwWindowHint(GLFW_RESIZABLE,      GLFW_FALSE);

        DebugOnly(glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE));

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
        glfwWindowHint(GLFW_OPENGL_PROFILE,         GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow* window = glfwCreateWindow(
            mode->width, 
            mode->height, 
            title, 
            fullscreen ? monitor : nullptr, 
            nullptr);
        DebugAssert(window);
        
        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if(!ms_gladInit)
        {
            i32 gladLoaded = gladLoadGL();
            DebugAssert(gladLoaded);
            ms_gladInit = true;
        }

        glfwSetMouseButtonCallback(window, 
            [](GLFWwindow* w, i32 btn, i32 action, i32 mods)
            {
                if(btn >= 0 && btn < 3)
                {
                    ImGui::GetIO().MouseDown[btn] = (action == GLFW_PRESS);
                }
                Ctrl::MouseButtonCB(btn, action, mods);
            });
        glfwSetCursorPosCallback(window, 
            [](GLFWwindow* w, f64 x, f64 y)
            {
                ImGui::GetIO().MousePos.x = (f32)x;
                ImGui::GetIO().MousePos.y = (f32)y;
                Ctrl::CursorPosCB((f32)x, (f32)y);
            });
        glfwSetScrollCallback(window, 
            [](GLFWwindow* w, f64 x, f64 y)
            {
                ImGui::GetIO().MouseWheel = (f32)y;
                Ctrl::ScrollWheelCB((f32)x, (f32)y);
            });
        glfwSetKeyCallback(window, 
            [](GLFWwindow* w, i32 key, i32 scancode, i32 action, i32 mods)
            {
                ImGuiIO& io = ImGui::GetIO();
                if(key >= 0 && key < 512)
                {
                    io.KeysDown[key] = action != GLFW_RELEASE;
                }
                io.KeyCtrl  = (0 != (mods & GLFW_MOD_CONTROL));
                io.KeyAlt   = (0 != (mods & GLFW_MOD_ALT));
                io.KeyShift = (0 != (mods & GLFW_MOD_SHIFT));
                Ctrl::KeyCB(key, action, mods);
            });
        glfwSetCharCallback(window, 
            [](GLFWwindow* w, uint32_t codepoint)
            {
                ImGui::GetIO().AddInputCharacter((ImWchar)codepoint);
            });

        i32 wwidth = 0;
        i32 wheight = 0;
        Window::GetSize({ window }, wwidth, wheight);
        glViewport(0, 0, wwidth, wheight);

#if _DEBUG
        i32 flags = 0; 
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        Assert(flags & GL_CONTEXT_FLAG_DEBUG_BIT);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
#endif // _DEBUG

        return { window };
    }

    void Destroy(WindowHandle window)
    {
        glfwDestroyWindow(ToWindow(window));

        --ms_glfwRefs;
        if(ms_glfwRefs == 0)
        {
            glfwTerminate();
        }
    }

    bool IsOpen(WindowHandle window)
    {
        return !glfwWindowShouldClose(ToWindow(window));
    }

    void SetShouldClose(WindowHandle window, bool closed)
    {
        glfwSetWindowShouldClose(ToWindow(window), closed);
    }

    void Swap(WindowHandle window)
    {
        glfwSwapBuffers(ToWindow(window));
    }

    void GetSize(WindowHandle window, i32& width, i32& height)
    {
        glfwGetWindowSize(ToWindow(window), &width, &height);
    }

    void GetCursorPos(WindowHandle window, double& xpos, double& ypos)
    {
        glfwGetCursorPos(ToWindow(window), &xpos, &ypos);
    }

    void SetCursorHidden(WindowHandle window, bool hidden)
    {
        glfwSetInputMode(
            ToWindow(window), 
            GLFW_CURSOR, 
            hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    WindowHandle GetActive()
    {
        return ms_active;
    }

    void SetActive(WindowHandle window)
    {
        ms_active = window;
    }

}; // Window
