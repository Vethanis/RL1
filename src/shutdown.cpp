#include "shutdown.h"

#include "macro.h"
#include "sokol_gfx.h"
#include "window.h"
#include "physics.h"

void Shutdown()
{
    sg_shutdown();
    
    Window* window = Window::GetActive();
    window->Shutdown();
}
