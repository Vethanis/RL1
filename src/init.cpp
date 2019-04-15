#include "init.h"

#include <time.h>
#include <stdlib.h>

#include "macro.h"
#include "allocator.h"
#include "window.h"
#include "camera.h"
#include "control.h"
#include "shader.h"
#include "physics.h"
#include "buffer.h"
#include "image.h"
#include "component.h"
#include "rendercomponent.h"
#include "task.h"
#include "prng.h"
#include "csg.h"
#include "ui.h"
#include "sokol_time.h"
#include "renderer.h"
#include "vkrenderer.h"
#include "vertex.h"

#include "stb_perlin.h"

static Camera defaultCam;

void Init()
{
    SRand(time(0) ^ (uint64_t)&puts);
    
    Window::SetActive(Window::Init("RL1", false));
    int32_t wwidth = 0;
    int32_t wheight = 0;
    Window::GetSize(Window::GetActive(), wwidth, wheight);
    defaultCam.Init(wwidth, wheight);
    defaultCam.move(vec3(0.0f, 0.0f, -3.0f));
    Camera::SetActive(&defaultCam);
    
    stm_setup();
    UI::Init();
    Control::Init();
    Components::Init();
    TaskManager::Init();
    Physics::Init();

    Renderer::Init();
}
