#include "ctrlbinding.h"
#include "control.h"
#include "system.h"
#include "sokol_time.h"

namespace CtrlBinding
{
    static u64 ms_lastTick = 0;
    static u64 ms_curTick = 0;

    void Init()
    {

    }
    void Update()
    {
        let after   = ms_curTick;
        ms_lastTick = after;
        ms_curTick  = stm_now();

        if (Ctrl::GetAfter(Key_Escape, after))
        {
            Systems::Quit();
            return;
        }
    }
    void Shutdown()
    {

    }
};
