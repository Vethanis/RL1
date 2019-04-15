#pragma once

#include "vector_types.h"

template<typename T>
struct Mat2 { Vec2<T> x, y;         Subscript(x); };

template<typename T>
struct Mat3 { Vec3<T> x, y, z;      Subscript(x); };

template<typename T>
struct Mat4 { Vec4<T> x, y, z, w;   Subscript(x); };

using float2x2  = Mat2<f32>;
using float3x3  = Mat3<f32>;
using float4x4  = Mat4<f32>;

using int2x2    = Mat2<i32>;
using int3x3    = Mat3<i32>;
using int4x4    = Mat4<i32>;

using uint2x2   = Mat2<u32>;
using uint3x3   = Mat3<u32>;
using uint4x4   = Mat4<u32>;

using bool2x2   = Mat2<bool>;
using bool3x3   = Mat3<bool>;
using bool4x4   = Mat4<bool>;
