#pragma once

#include "component_types.h"
#include "memory.h"

using ComponentNewFn  = void(*)(ComponentData component);
using ComponentDropFn = void(*)(ComponentData component);

// ----------------------------------------------------------------------------

inline void EraseComp(ComponentData component)
{
    EraseP(component.begin(), component.size());
}

inline void FloatFill1(ComponentData comp)
{
    FloatSlice floats = ToFloats(comp);
    for(f32& x : floats)
    {
        x = 1.0f;
    }
}

inline void FloatFill0(ComponentData comp)
{
    FloatSlice floats = ToFloats(comp);
    for(f32& x : floats)
    {
        x = 0.0f;
    }
}

// ----------------------------------------------------------------------------

inline void AABBNew(ComponentData comp)
{
    Slice<AABB> aabb = ToType<AABB>(comp);
    aabb[0].lo = { -0.5f, -0.5f, -0.5f };
    aabb[0].hi = {  0.5f,  0.5f,  0.5f };
}

inline void SphereNew(ComponentData comp)
{
    Slice<Sphere> sphere = ToType<Sphere>(comp);
    sphere[0].center = { 0.0f, 0.0f, 0.0f };
    sphere[0].radius = 1.0f;
}

inline void PlaneNew(ComponentData comp)
{
    Slice<Plane> plane = ToType<Plane>(comp);
    plane[0].normal = { 0.0f, 1.0f, 0.0f };
    plane[0].distance = 0.0f;
}

// ----------------------------------------------------------------------------

static constexpr ComponentNewFn sc_ComponentNew[] =
{
    FloatFill0, // CT_Position
    FloatFill0, // CT_Orientation
    FloatFill1, // CT_Scale
    FloatFill0, // CT_Velocity
    FloatFill1, // CT_Mass
    AABBNew,    // CT_AABB
    SphereNew,  // CT_Sphere
    PlaneNew,   // CT_Plane
    EraseComp,  // CT_Name
    EraseComp,  // CT_NameHash
    EraseComp,  // CT_Inventory
    FloatFill0, // CT_Health
};
CountAssert(sc_ComponentNew, CT_Count);

// ----------------------------------------------------------------------------

inline void InventoryDrop(ComponentData comp)
{
    Inventory* pInventory = (Inventory*)comp.begin();
    pInventory->ids.reset();
}

// ----------------------------------------------------------------------------

static constexpr ComponentDropFn sc_ComponentDrop[] =
{
    EraseComp,      // CT_Position
    EraseComp,      // CT_Orientation
    EraseComp,      // CT_Scale
    EraseComp,      // CT_Velocity
    EraseComp,      // CT_Mass
    EraseComp,      // CT_AABB
    EraseComp,      // CT_Sphere
    EraseComp,      // CT_Plane
    EraseComp,      // CT_Name
    EraseComp,      // CT_NameHash
    InventoryDrop,  // CT_Inventory
    EraseComp,      // CT_Health
};
CountAssert(sc_ComponentDrop, CT_Count);
