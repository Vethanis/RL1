#include "window.h"

#include <stdio.h>
#include "macro.h"
#include "control.h"
#include "imgui.h"

#if VK_BACKEND
    #define GLFW_INCLUDE_VULKAN
    #include <GLFW/glfw3.h>
#else
    #define GLFW_INCLUDE_NONE
    #include <GLFW/glfw3.h>
    #include "glad.h"
#endif // VK_BACKEND

namespace Window
{
    static int32_t      ms_glfwRefs = 0;
    static GLFWwindow*  ms_active = nullptr;

    GL_ONLY(
        static bool ms_gladInit = false;
        );

    DGL_ONLY(
    /*
        typedef void (APIENTRY *GLDEBUGPROC)(
            GLenum source,
            GLenum type,
            GLuint id,
            GLenum severity,
            GLsizei length,
            const GLchar *message,
            const void *userParam);
    */

        void APIENTRY glDebugOutput(
            GLenum source, 
            GLenum type, 
            GLuint id, 
            GLenum severity, 
            GLsizei length, 
            const GLchar *message, 
            const void *userParam)
        {
            if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

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

            Assert(false);
        }
    );

    GLFWwindow* Init(const char* title, bool fullscreen)
    {
        ++ms_glfwRefs;
        if(ms_glfwRefs == 1)
        {
            int32_t glfwLoaded = glfwInit();
            Assert(glfwLoaded);
        }

        glfwSetErrorCallback(
            [](int32_t error, const char* msg)
            {
                fprintf(stderr, "%d :: %s\n", error, msg);
            });

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        Assert(monitor);

        const GLFWvidmode* mode = glfwGetVideoMode(monitor);
        Assert(mode);
        
        glfwWindowHint(GLFW_RED_BITS,       mode->redBits);
        glfwWindowHint(GLFW_GREEN_BITS,     mode->greenBits);
        glfwWindowHint(GLFW_BLUE_BITS,      mode->blueBits);
        glfwWindowHint(GLFW_REFRESH_RATE,   mode->refreshRate);
        glfwWindowHint(GLFW_RESIZABLE,      GLFW_FALSE);

        DGL_ONLY(
            glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
            );

        GL_ONLY(
            glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  4);
            glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
            glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
            glfwWindowHint(GLFW_OPENGL_PROFILE,         GLFW_OPENGL_CORE_PROFILE);
        );

        VK_ONLY(glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API));

        GLFWwindow* window = glfwCreateWindow(
            mode->width, 
            mode->height, 
            title, 
            fullscreen ? monitor : nullptr, 
            nullptr);
        Assert(window);
        
        GL_ONLY(
            glfwMakeContextCurrent(window);
            glfwSwapInterval(1);
            );

        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        GL_ONLY(
            if(!ms_gladInit)
            {
                int32_t gladLoaded = gladLoadGL();
                Assert(gladLoaded);
                ms_gladInit = true;
            });

        glfwSetMouseButtonCallback(window, 
            [](GLFWwindow* w, int32_t btn, int32_t action, int32_t mods)
            {
                if(btn >= 0 && btn < 3)
                {
                    ImGui::GetIO().MouseDown[btn] = (action == GLFW_PRESS);
                }
                Control::MouseButtonCB(btn, action, mods);
            });
        glfwSetCursorPosCallback(window, 
            [](GLFWwindow* w, double x, double y)
            {
                ImGui::GetIO().MousePos.x = (float)x;
                ImGui::GetIO().MousePos.y = (float)y;
            });
        glfwSetScrollCallback(window, 
            [](GLFWwindow* w, double x, double y)
            {
                ImGui::GetIO().MouseWheel = (float)y;
                Control::ScrollWheelCB((float)x, (float)y);
            });
        glfwSetKeyCallback(window, 
            [](GLFWwindow* w, int32_t key, int32_t scancode, int32_t action, int32_t mods)
            {
                ImGuiIO& io = ImGui::GetIO();
                if(key >= 0 && key < 512)
                {
                    io.KeysDown[key] = action != GLFW_RELEASE;
                }
                io.KeyCtrl  = (0 != (mods & GLFW_MOD_CONTROL));
                io.KeyAlt   = (0 != (mods & GLFW_MOD_ALT));
                io.KeyShift = (0 != (mods & GLFW_MOD_SHIFT));
                Control::KeyCB(key, action, mods);
            });
        glfwSetCharCallback(window, 
            [](GLFWwindow* w, uint32_t codepoint)
            {
                ImGui::GetIO().AddInputCharacter((ImWchar)codepoint);
            });

        GL_ONLY(
            int32_t wwidth = 0;
            int32_t wheight = 0;
            Window::GetSize(window, wwidth, wheight);
            glViewport(0, 0, wwidth, wheight);
            );

        DGL_ONLY(
            int32_t flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
            Assert(flags & GL_CONTEXT_FLAG_DEBUG_BIT);
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback(glDebugOutput, nullptr);
            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
        );

        return window;
    }

    void Shutdown(GLFWwindow* window)
    {
        Assert(window);
        glfwDestroyWindow(window);

        --ms_glfwRefs;
        if(ms_glfwRefs == 0)
        {
            glfwTerminate();
        }
    }

    bool IsOpen(GLFWwindow* window)
    {
        Assert(window);
        return !glfwWindowShouldClose(window);
    }

    void SetShouldClose(GLFWwindow* window, bool closed)
    {
        Assert(window);
        glfwSetWindowShouldClose(window, closed);
    }

    void Swap(GLFWwindow* window)
    {
        Assert(window);
        glfwSwapBuffers(window);
    }

    void GetSize(GLFWwindow* window, int32_t& width, int32_t& height)
    {
        Assert(window);
        glfwGetWindowSize(window, &width, &height);
    }

    void GetCursorPos(GLFWwindow* window, double& xpos, double& ypos)
    {
        Assert(window);
        glfwGetCursorPos(window, &xpos, &ypos);
    }

    void SetCursorHidden(GLFWwindow* window, bool hidden)
    {
        Assert(window);
        glfwSetInputMode(
            window, 
            GLFW_CURSOR, 
            hidden ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
    }

    GLFWwindow* GetActive()
    {
        Assert(ms_active);
        return ms_active;
    }

    void SetActive(GLFWwindow* window)
    {
        Assert(window);
        ms_active = window;
    }

}; // Window
