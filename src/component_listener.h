#pragma once

#include "slot.h"

struct EntityDestructor
{
    void (*fn)(void*, slot);
    void* data;

    inline bool operator==(const EntityDestructor& other) const 
    {
        return fn == other.fn && data == other.data;
    }
};

void EntityDestroyed(slot s);
void RegisterDestructor(EntityDestructor d);
