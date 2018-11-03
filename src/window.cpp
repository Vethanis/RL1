#include "window.h"

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include "glad.h"

#include "macro.h"
#include "camera.h"

static int32_t  glfwRefs = 0;
static bool     gladInit = false;

static void error_callback(
    int32_t     error, 
    const char* desc)
{
    fprintf(stderr, "%s\n", desc);
}

void Window::Init(const char* title, bool fullscreen)
{
    ++glfwRefs;
    if(glfwRefs == 1)
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

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,  3);
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

    if(!gladInit)
    {
        int32_t gladLoaded = gladLoadGL();
        Assert(gladLoaded);
        gladInit = true;
    }

    glViewport(0, 0, m_width, m_height);
}

void Window::Shutdown()
{
    glfwDestroyWindow(m_window);

    --glfwRefs;
    if(glfwRefs == 0)
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
        v.z -= glfwGetKey(m_window, GLFW_KEY_W)             ? 1.0f : 0.0f;
        v.z += glfwGetKey(m_window, GLFW_KEY_S)             ? 1.0f : 0.0f;
        v.x -= glfwGetKey(m_window, GLFW_KEY_A)             ? 1.0f : 0.0f;
        v.x += glfwGetKey(m_window, GLFW_KEY_D)             ? 1.0f : 0.0f;
        v.y += glfwGetKey(m_window, GLFW_KEY_SPACE)         ? 1.0f : 0.0f;
        v.y -= glfwGetKey(m_window, GLFW_KEY_LEFT_SHIFT)    ? 1.0f : 0.0f;
    }

    cam.move(v * m_dt);
    cam.yaw(m_dcx * m_dt * 1000.0f);
    cam.pitch(m_dcy * m_dt * 1000.0f);
}
