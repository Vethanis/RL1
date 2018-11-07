#pragma once

#include "component.h"

struct ComponentAllocator
{
    virtual ~ComponentAllocator(){}
    virtual Component* Alloc() = 0;
    virtual void Free(Component* c) = 0;
};
