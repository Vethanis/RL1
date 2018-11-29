#include "shutdown.h"

#include "macro.h"
#include "sokol_gfx.h"
#include "window.h"
#include "physics.h"
#include "task.h"

void Shutdown()
{
    TaskManager::Shutdown();
    Physics::Shutdown();

    sg_shutdown();
    
    Window* window = Window::GetActive();
    window->Shutdown();
}
