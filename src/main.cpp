
#include "window.h"
#include "system.h"
#include "sokol_time.h"

int main()
{
    stm_setup();
    
    for(const auto fn : sc_SystemInits)
    {
        fn();
    }
    while(Window::IsOpen())
    {
        for(const auto fn : sc_SystemUpdates)
        {
            fn();
        }
    }
    for(const auto fn : sc_SystemShutdowns)
    {
        fn();
    }
}
