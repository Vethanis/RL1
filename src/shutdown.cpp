#include "shutdown.h"

#include "macro.h"
#include "renderer.h"
#include "window.h"
#include "physics.h"
#include "task.h"
#include "ui.h"
#include "control.h"

void Shutdown()
{
    TaskManager::Shutdown();
    Physics::Shutdown();
    UI::Shutdown();
    Control::Shutdown();
    Renderer::Shutdown();
    Window::GetActive()->Shutdown();
}
