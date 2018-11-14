
#include "window.h"

#include "init.h"
#include "update.h"
#include "draw.h"
#include "shutdown.h"

int main()
{
    Init();
    while(Window::GetActive()->Open())
    {
        Update();
        Draw();
    }
    Shutdown();
}
