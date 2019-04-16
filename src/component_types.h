#pragma once

#include "hlsl_types.h"
#include "array.h"
#include "bitfield.h"

struct Position
{
    float3 position;
};

struct Orientation
{
    float4 orientation;
};

struct Scale
{
    float3 scale;
};

struct Velocity
{
    float3 velocity;
};

struct Mass
{
    f32 mass;
};

struct AABB
{
    float3 lo;
    float3 hi;
};

struct Sphere
{
    float3  center;
    f32     radius;
};

struct Plane
{
    float3  normal;
    f32     distance;
};

struct Name
{
    char str[64];
};

struct NameHash
{
    u64 hash;
};

struct ItemID
{
    u32 value;
};

struct Inventory
{
    Array<ItemID> ids;
};

struct Health
{
    f32 value;
};

// ----------------------------------------------------------------------------

enum ComponentType : u32
{
    CT_Position = 0,
    CT_Orientation,
    CT_Scale,
    CT_Velocity,
    CT_Mass,
    CT_AABB,
    CT_Sphere,
    CT_Plane,
    CT_Name,
    CT_NameHash,
    CT_Inventory,
    CT_Health,

    CT_Count
};

using ComponentData = Slice<u8>;
using ComponentFlags = BitField<CT_Count>;

// ----------------------------------------------------------------------------

static constexpr size_t sc_ComponentSize[] =
{
    sizeof(Position),
    sizeof(Orientation),
    sizeof(Scale),
    sizeof(Velocity),
    sizeof(Mass),
    sizeof(AABB),
    sizeof(Sphere),
    sizeof(Plane),
    sizeof(Name),
    sizeof(NameHash),
    sizeof(Inventory),
    sizeof(Health),
};
CountAssert(sc_ComponentSize, CT_Count);

// ----------------------------------------------------------------------------

template<typename T>
static constexpr ComponentType GetComponentType();

template<>
static constexpr ComponentType GetComponentType<Position>()     { return CT_Position;       }
template<>
static constexpr ComponentType GetComponentType<Orientation>()  { return CT_Orientation;    }
template<>
static constexpr ComponentType GetComponentType<Scale>()        { return CT_Scale;          }
template<>
static constexpr ComponentType GetComponentType<Velocity>()     { return CT_Velocity;       }
template<>
static constexpr ComponentType GetComponentType<Mass>()         { return CT_Mass;           }
template<>
static constexpr ComponentType GetComponentType<AABB>()         { return CT_AABB;           }
template<>
static constexpr ComponentType GetComponentType<Sphere>()       { return CT_Sphere;         }
template<>
static constexpr ComponentType GetComponentType<Plane>()        { return CT_Plane;          }
template<>
static constexpr ComponentType GetComponentType<Name>()         { return CT_Name;           }
template<>
static constexpr ComponentType GetComponentType<NameHash>()     { return CT_NameHash;       }
template<>
static constexpr ComponentType GetComponentType<Inventory>()    { return CT_Inventory;      }
template<>
static constexpr ComponentType GetComponentType<Health>()       { return CT_Health;         }
