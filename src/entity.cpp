#include "entity.h"

#include "macro.h"
#include "array.h"
#include "slots.h"
#include "component.h"

Array<slot>     ms_slots;
Array<int32_t>  ms_free;
Array<int32_t>  ms_live;

Entity Entities::Create()
{
    slot s = {0};

    if(!ms_free.empty())
    {
        s.id = ms_free.back();
        s.gen = ms_slots[s.id].gen;
        ms_free.pop();
    }
    else
    {
        s.id = ms_slots.count() - 1;
        s.gen = 0u;
        ms_slots.grow() = s;
    }

    ms_live.grow() = s.id;

    return s;
}

void Entities::Destroy(Entity s)
{
    Assert(Alive(s));

    Component::RemoveAll(s);

    ms_slots[s.id].gen++;
    ms_free.grow() = s.id;
    ms_live.findRemove(s.id);
}

bool Entities::Alive(slot s) 
{ 
    return s.id < ms_slots.count() && s.gen == ms_slots[s.id].gen; 
}

int32_t Entities::Count() 
{ 
    return ms_live.count(); 
}

Entity Entities::Get(int32_t i)
{
    int32_t idx = ms_live[i];
    return ms_slots[idx];
}
