
#include "window.h"

#include "init.h"
#include "update.h"
#include "draw.h"
#include "shutdown.h"
#include "sokol_time.h"

int main()
{
    Init();
    uint64_t last_time = stm_now();
    while(Window::IsOpen(Window::GetActive()))
    {
        uint64_t dt = stm_laptime(&last_time);
        Update(
            (float)stm_sec(last_time), 
            (float)stm_sec(dt));
        Draw();
    }
    Shutdown();
}
