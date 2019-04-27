#pragma once

#include "component_types_thin.h"
#include "hlsl_types.h"
#include "array.h"

struct Position
{
    float3 value;
};

struct Orientation
{
    float3 forward;
    float3 up;
};

struct Scale
{
    float2 value;
};

struct Velocity
{
    float3 value;
};

struct Mass
{
    f32 value;
};

struct NameHash
{
    u64 value;
};

struct DrawInfo
{
    u32 id;     // which texture
    f32 phase;  // position in texture animation
    f32 hz;     // animation loops per second
};

struct Health
{
    f32 value;
};

// ----------------------------------------------------------------------------

static constexpr usize sc_ComponentSize[] =
{
    sizeof(Position),
    sizeof(Orientation),
    sizeof(Scale),
    sizeof(Velocity),
    sizeof(Mass),
    sizeof(NameHash),
    sizeof(DrawInfo),
    sizeof(Health),
};
CountAssert(sc_ComponentSize, CT_Count);

// ----------------------------------------------------------------------------

#define GCT(T, id) \
    template<> \
    inline constexpr ComponentType GetComponentType<T>() { return id; }

GCT(Position, CT_Position)
GCT(Orientation, CT_Orientation)
GCT(Scale, CT_Scale)
GCT(Velocity, CT_Velocity)
GCT(Mass, CT_Mass)
GCT(NameHash, CT_NameHash)
GCT(DrawInfo, CT_DrawInfo)
GCT(Health, CT_Health)

#undef GCT
