#pragma once

#include "lang.h"
#include "slice.h"

struct Entity
{
    Slot s;
};

enum ComponentType : u32
{
    CT_Position = 0,
    CT_Orientation,
    CT_Scale,
    CT_Velocity,
    CT_Mass,
    CT_NameHash,
    CT_DrawInfo,
    CT_Health,

    CT_Count
};

using ComponentFlags    = u32;
using ComponentData     = Slice<u8>;
using CComponentData    = Slice<const u8>;

template<typename T>
inline constexpr ComponentType GetComponentType();
