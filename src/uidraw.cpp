#include "uidraw.h"

#include "imguishim.h"
#include "window.h"

void UIBegin()
{
    ImGuiShim::Begin(Window::GetActive()->m_window);
}

void UIEnd()
{
    ImGuiShim::End();
}