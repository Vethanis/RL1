#pragma once

#include <stdint.h>
#include "slot.h"

typedef slot Entity;

namespace Entities
{
    Entity Create();
    void Destroy(Entity s);
    bool Alive(slot s);
    int32_t Count();
    Entity Get(int32_t i);
};
