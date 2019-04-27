#pragma once

#include "macro.h"
#include "component_types.h"
#include "memory.h"
#include "ecs.h"

inline void EraseComp(ComponentData comp)
{
    EraseP(comp.begin(), comp.size());
}

inline void FloatFill1(ComponentData comp)
{
    for(f32& x : ToFloats(comp))
    {
        x = 1.0f;
    }
}

inline void FloatFill0(ComponentData comp)
{
    for(f32& x : ToFloats(comp))
    {
        x = 0.0f;
    }
}

inline void OrientationNew(ComponentData comp)
{
    Orientation& ori = ToType<Orientation>(comp)[0];
    ori.forward = { 0.0f, 0.0f, 1.0f };
    ori.up = { 0.0f, 1.0f, 0.0f };
}

inline void DrawInfoNew(ComponentData comp)
{
    DrawInfo& info = ToType<DrawInfo>(comp)[0];
    info.hz = 1.0f;
    info.phase = 0.0f;
    info.id = 0;
}

inline void HealthNew(ComponentData comp)
{
    Health& health = ToType<Health>(comp)[0];
    health.value = 100.0f;
}

// ----------------------------------------------------------------------------

def ComponentNewFn sc_ComponentNew[] =
{
    FloatFill0,     // CT_Position
    OrientationNew, // CT_Orientation
    FloatFill1,     // CT_Scale
    FloatFill0,     // CT_Velocity
    FloatFill1,     // CT_Mass
    EraseComp,      // CT_NameHash
    DrawInfoNew,    // CT_DrawInfo
    HealthNew,      // CT_Health
};
CountAssert(sc_ComponentNew, CT_Count);

// ----------------------------------------------------------------------------

def ComponentDropFn sc_ComponentDrop[] =
{
    EraseComp,      // CT_Position
    EraseComp,      // CT_Orientation
    EraseComp,      // CT_Scale
    EraseComp,      // CT_Velocity
    EraseComp,      // CT_Mass
    EraseComp,      // CT_NameHash
    EraseComp,      // CT_DrawInfo
    EraseComp,      // CT_Health
};
CountAssert(sc_ComponentDrop, CT_Count);
