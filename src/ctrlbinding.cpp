#include "ctrlbinding.h"
#include "control.h"

namespace CtrlBinding
{
    static constexpr float OneFrame = 1.0f / 60.0f;
    void Init()
    {
        
    }
    void Update()
    {
        Ctrl::Event evt = {};
        if (Ctrl::Get(Ctrl::Key_ESCAPE, OneFrame, evt))
        {
            Ctrl::CloseMainWindow();
        }
    }
    void Shutdown()
    {

    }
};
