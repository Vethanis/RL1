
#include "window.h"
#include "system.h"

int main()
{
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
