#include "component_listener.h"

#include "array.h"

static Array<EntityDestructor> destructors;

void EntityDestroyed(slot s)
{
    for(EntityDestructor& d : destructors)
    {
        d.fn(d.data, s);
    }
}

void RegisterDestructor(EntityDestructor d)
{
    destructors.uniquePush(d);
}
