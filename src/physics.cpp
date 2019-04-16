#include "physics.h"
#include "macro.h"
#include "sokol_time.h"
#include "ecs.h"

namespace Physics
{
    static u64 ms_time = 0;

    void Init()
    {

    }
    void Update()
    {
        float dt = stm_sec(stm_laptime(&ms_time));

        Slice<Position> positions = ECS::GetAll<Position>();
    }
    void Shutdown()
    {

    }
};

