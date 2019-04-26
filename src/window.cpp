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

static void SetCallbacks(GLFWwindow* window)
{
    glfwSetWindowSizeCallback(window, [](GLFWwindow* w, i32 width, i32 height)
        {
            Ctrl::WindowSizeCB(width, height);
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
}

namespace Window
{
    static GLFWwindow*  ms_window;
    static bool         ms_fullscreen = false;

    void Init()
    {
        glfwSetErrorCallback(
            [](i32 error, cstr msg)
            {
                puts(msg);
                DebugInterrupt();
            });

        VerifyEQ(glfwInit(), GLFW_TRUE);

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        DebugAssert(monitor);

        let mode = glfwGetVideoMode(monitor);
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
            "RL1",
            ms_fullscreen ? monitor : nullptr,
            nullptr);
        DebugAssert(window);

        SetCallbacks(window);

        glfwMakeContextCurrent(window);
        glfwSwapInterval(1);

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        VerifyEQ(gladLoadGL(), 1);

        Ctrl::WindowSizeCB(mode->width, mode->height);
        glViewport(0, 0, mode->width, mode->height);

#if _DEBUG
        i32 flags = 0;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        Assert(flags & GL_CONTEXT_FLAG_DEBUG_BIT);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
#endif // _DEBUG

        ms_window = window;
    }

    void Update()
    {
        glfwSwapBuffers(ms_window);
    }

    void Shutdown()
    {
        glfwDestroyWindow(ms_window);
        glfwTerminate();
    }

    bool IsOpen()
    {
        return !glfwWindowShouldClose(ms_window);
    }

    void SetShouldClose(bool closed)
    {
        glfwSetWindowShouldClose(ms_window, closed);
    }

    void SetCursorHidden(bool hidden)
    {
        glfwSetInputMode(
            ms_window,
            GLFW_CURSOR,
            hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    GLFWwindow* Get()
    {
        return ms_window;
    }

}; // Window
