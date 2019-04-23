#pragma once

#include "macro.h"

#define Subscript(x)                                                    \
    inline       auto& operator[](u32 i)       { return (&x)[i]; }   \
    inline const auto& operator[](u32 i) const { return (&x)[i]; }

template<typename T>
struct alignas( 8) Vec2 { T x, y;       Subscript(x); };

template<typename T>
struct alignas(16) Vec3 { T x, y, z;    Subscript(x); };

template<typename T>
struct alignas(16) Vec4 { T x, y, z, w; Subscript(x); };

using float2    = Vec2<f32>;
using float3    = Vec3<f32>;
using float4    = Vec4<f32>;

using int2      = Vec2<i32>;
using int3      = Vec3<i32>;
using int4      = Vec4<i32>;

using uint2     = Vec2<u32>;
using uint3     = Vec3<u32>;
using uint4     = Vec4<u32>;

using bool2     = Vec2<bool>;
using bool3     = Vec3<bool>;
using bool4     = Vec4<bool>;
