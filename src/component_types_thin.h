#pragma once

#include "macro.h"

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

struct ComponentFlags
{
    u32 flags[DivCeil((u32)CT_Count, 32u)];

    inline bool has(u32 type) const
    {
        DebugAssert(type < CT_Count);
        u32 dword = type >> 5u;
        u32 bit = (u32)(type) & (31u);
        return (flags[dword] >> bit) & 1u;
    }
    inline void set(u32 type)
    {
        DebugAssert(type < CT_Count);
        u32 dword = type >> 5u;
        u32 bit = (u32)(type) & (31u);
        u32 mask = (1u << bit);
        flags[dword] |= mask;
    }
    inline void unset(u32 type)
    {
        DebugAssert(type < CT_Count);
        u32 dword = type >> 5u;
        u32 bit = (u32)(type) & (31u);
        u32 mask = (1u << bit);
        flags[dword] &= ~mask;
    }
};

using ComponentData     = Slice<u8>;
using CComponentData    = Slice<const u8>;
using ComponentNewFn    = void(*)(ComponentData component);
using ComponentDropFn   = void(*)(ComponentData component);

template<typename T>
inline constexpr ComponentType GetComponentType();
