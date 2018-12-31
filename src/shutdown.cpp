#include "shutdown.h"

#include "macro.h"
#include "renderer.h"
#include "window.h"
#include "physics.h"
#include "task.h"
#include "ui.h"

void Shutdown()
{
    TaskManager::Shutdown();
    Physics::Shutdown();
    UI::Shutdown();
    Renderer::Shutdown();
    Window::GetActive()->Shutdown();
}
