#include "window.h"

#include <stdio.h>

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"
#include "imgui.h"

#include "macro.h"
#include "camera.h"

static int32_t  ms_glfwRefs = 0;
static bool     ms_gladInit = false;
static bool     ms_cursorHidden = true;
static Window*  ms_active = nullptr;
static int32_t  ms_frame = 0;

static void error_callback(
    int32_t     error, 
    const char* desc)
{
    fprintf(stderr, "%s\n", desc);
}

#if DEBUG_GL

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
    }
#endif // DEBUG_GL

void Window::Init(const char* title, bool fullscreen)
{
    memset(this, 0, sizeof(*this));

    ++ms_glfwRefs;
    if(ms_glfwRefs == 1)
    {
        int32_t glfwLoaded = glfwInit();
        Assert(glfwLoaded);
    }

    glfwSetErrorCallback(error_callback);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    Assert(monitor);

    const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    Assert(mode);
    
    glfwWindowHint(GLFW_RED_BITS,       mode->redBits);
    glfwWindowHint(GLFW_GREEN_BITS,     mode->greenBits);
    glfwWindowHint(GLFW_BLUE_BITS,      mode->blueBits);
    glfwWindowHint(GLFW_REFRESH_RATE,   mode->refreshRate);

    #if DEBUG_GL
        glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    #endif // DEBUG_GL
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,  3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,  GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE,         GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(
        mode->width, 
        mode->height, 
        title, 
        fullscreen ? monitor : nullptr, 
        nullptr);
    Assert(m_window);
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);
    glfwGetWindowSize(m_window, &m_width, &m_height);
    glfwSetInputMode(m_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if(!ms_gladInit)
    {
        int32_t gladLoaded = gladLoadGL();
        Assert(gladLoaded);
        ms_gladInit = true;
    }

    glfwSetMouseButtonCallback(m_window, 
        [](GLFWwindow* w, int32_t btn, int32_t action, int32_t mods)
        {
            if(btn >= 0 && btn < 3)
            {
                ImGui::GetIO().MouseDown[btn] = (action == GLFW_PRESS);
            }
        });
    glfwSetCursorPosCallback(m_window, 
        [](GLFWwindow* w, double x, double y)
        {
            ImGui::GetIO().MousePos.x = (float)x;
            ImGui::GetIO().MousePos.y = (float)y;
        });
    glfwSetScrollCallback(m_window, 
        [](GLFWwindow* w, double x, double y)
        {
            ImGui::GetIO().MouseWheel = (float)y;
        });
    glfwSetKeyCallback(m_window, 
        [](GLFWwindow* w, int32_t key, int32_t scancode, int32_t action, int32_t mods)
        {
            ImGuiIO& io = ImGui::GetIO();
            if(key >= 0 && key < 512)
            {
                io.KeysDown[key] = (action == GLFW_PRESS) || (action == GLFW_REPEAT);
            }
            io.KeyCtrl  = (0 != (mods & GLFW_MOD_CONTROL));
            io.KeyAlt   = (0 != (mods & GLFW_MOD_ALT));
            io.KeyShift = (0 != (mods & GLFW_MOD_SHIFT));

            if(key == GLFW_KEY_LEFT_CONTROL && action == GLFW_RELEASE)
            {
                ms_cursorHidden = !ms_cursorHidden;
                if(ms_cursorHidden)
                {
                    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
                }
                else
                {
                    glfwSetInputMode(w, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
                }
            }
        });
    glfwSetCharCallback(m_window, 
        [](GLFWwindow* w, uint32_t codepoint)
        {
            ImGui::GetIO().AddInputCharacter((ImWchar)codepoint);
        });

    glViewport(0, 0, m_width, m_height);

    #if DEBUG_GL
        int32_t flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        Assert(flags & GL_CONTEXT_FLAG_DEBUG_BIT);
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        glDebugMessageCallback(glDebugOutput, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE);
    #endif //DEBUG_GL
}

void Window::Shutdown()
{
    glfwDestroyWindow(m_window);

    --ms_glfwRefs;
    if(ms_glfwRefs == 0)
    {
        glfwTerminate();
    }
}

bool Window::Open()
{
    return !glfwWindowShouldClose(m_window);
}

void Window::Swap()
{
    glfwSwapBuffers(m_window);
}

void Window::Poll(Camera& cam)
{
    glfwPollEvents();

    if(glfwGetKey(m_window, GLFW_KEY_ESCAPE))
    {
        glfwSetWindowShouldClose(m_window, true);
    }

    {
        m_dt = (float)glfwGetTime();
        glfwSetTime(0.0);
        m_dt = glm::clamp(m_dt, 0.0f, 1.0f / 30.0f);
    }

    if(!ms_cursorHidden)
    {
        ms_frame = 0;
        return;
    }

    {
        double xpos, ypos;
        glfwGetCursorPos(m_window, &xpos, &ypos);
        glfwGetWindowSize(m_window, &m_width, &m_height);
        float ncx = (float)xpos / (float)m_width;
        float ncy = (float)ypos / (float)m_height;
        ncx = ncx * 2.0f - 1.0f;
        ncy = ncy * 2.0f - 1.0f;
        ncx = -ncx;
        ncy = -ncy;
        m_dcx = ncx - m_cx;
        m_dcy = ncy - m_cy;
        m_cx = ncx;
        m_cy = ncy;
    }

    glm::vec3 v(0.0f);
    {
        v.z += glfwGetKey(m_window, GLFW_KEY_W)             ? 1.0f : 0.0f;
        v.z -= glfwGetKey(m_window, GLFW_KEY_S)             ? 1.0f : 0.0f;
        v.x -= glfwGetKey(m_window, GLFW_KEY_A)             ? 1.0f : 0.0f;
        v.x += glfwGetKey(m_window, GLFW_KEY_D)             ? 1.0f : 0.0f;
        v.y += glfwGetKey(m_window, GLFW_KEY_SPACE)         ? 1.0f : 0.0f;
        v.y -= glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT)    ? 1.0f : 0.0f;
    }

    cam.resize(m_width, m_height);
    if(ms_frame != 0)
    {
        // first frame has a large offset we want to skip
        cam.move(v * m_dt);
        cam.yaw(m_dcx * m_dt * 1000.0f);
        cam.pitch(m_dcy * m_dt * 1000.0f);
    }

    ++ms_frame;
}

Window* Window::GetActive()
{
    return ms_active;
}

void Window::SetActive(Window* window)
{
    ms_active = window;
}
