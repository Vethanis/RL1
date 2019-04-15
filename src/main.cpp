
#include "window.h"

#include "init.h"
#include "update.h"
#include "draw.h"
#include "shutdown.h"
#include "sokol_time.h"

int main()
{
    Init();
    u64 last_time = stm_now();
    while(Window::IsOpen(Window::GetActive()))
    {
        u64 dt = stm_laptime(&last_time);
        Update(
            (f32)stm_sec(last_time), 
            (f32)stm_sec(dt));
        Draw();
    }
    Shutdown();
}
