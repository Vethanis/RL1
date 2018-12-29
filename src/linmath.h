#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/euler_angles.hpp>

typedef glm::mat4 mat4;
typedef glm::mat3 mat3;
typedef glm::mat2 mat2;
typedef glm::vec4 vec4;
typedef glm::vec3 vec3;
typedef glm::vec2 vec2;
typedef glm::ivec4 ivec4;
typedef glm::ivec3 ivec3;
typedef glm::ivec2 ivec2;
typedef glm::uvec4 uvec4;
typedef glm::uvec3 uvec3;
typedef glm::uvec2 uvec2;
typedef glm::quat quat;
typedef glm::bvec2 bvec2;
typedef glm::bvec3 bvec3;
typedef glm::bvec4 bvec4;

#define UP      vec3(0.0f, 1.0f, 0.0f)
#define FORWARD vec3(0.0f, 0.0f, -1.0f)
#define RIGHT   vec3(1.0f, 0.0f, 0.0f)

inline vec3 getRight(const mat4& m)
{
    return vec3(m[0][0], m[1][0], m[2][0]);
}

inline vec3 getUp(const mat4& m)
{
    return vec3(m[0][1], m[1][1], m[2][1]);
}

inline vec3 getForward(const mat4& m)
{
    return vec3(-m[0][2], -m[1][2], -m[2][2]);
}

inline vec3 getRight(const quat& q)
{
    return q * RIGHT;
}

inline vec3 getUp(const quat& q)
{
    return q * UP;
}

inline vec3 getForward(const quat& q)
{
    return q * FORWARD;
}

inline void toAxisAngle(const quat& qIn, vec3& axis, float& angle)
{
    quat q = glm::normalize(qIn);
    angle = 2.0f * acosf(q.w);
    float s = sqrtf(1.0f - q.w * q.w);
    if(s < 0.001f)
    {
        axis.x = 0.0f;
        axis.y = 0.0f;
        axis.z = -1.0f;
    }
    else
    {
        axis.x = q.x / s;
        axis.y = q.y / s;
        axis.z = q.z / s;
    }
}

// quats are applied right to left
inline quat Rotate(const quat& rotator, const quat& base)
{
    return base * rotator;
}

inline vec3 Rotate(const quat& a, const vec3& b)
{
    return a * b;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> GT(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] > b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> GTE(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] >= b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> LT(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] < b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> LTE(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] <= b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> EQ(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] == b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> NEQ(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = a[i] != b[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> NEAR(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b, T eps)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = (glm::max(a[i], b[i]) - glm::min(a[i], b[i])) < eps;
    }
    return y;
}

template<int32_t dim, typename T>
inline glm::vec<dim, bool> FAR(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b, T eps)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = (glm::max(a[i], b[i]) - glm::min(a[i], b[i])) > eps;
    }
    return y;
}

template<int32_t dim>
inline glm::vec<dim, bool> NOT(const glm::vec<dim, bool>& x)
{
    glm::vec<dim, bool> y;
    for(int32_t i = 0; i < dim; ++i)
    {
        y[i] = ~x[i];
    }
    return y;
}

template<int32_t dim>
bool AND(const glm::vec<dim, bool>& x)
{
    bool y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y = y & x[i];
    }
    return y;
}

template<int32_t dim>
bool NAND(const glm::vec<dim, bool>& x)
{
    bool y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y = ~(y & x[i]);
    }
    return y;
}

template<int32_t dim>
bool OR(const glm::vec<dim, bool>& x)
{
    bool y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y |= x[i];
    }
    return y;
}

template<int32_t dim>
bool NOR(const glm::vec<dim, bool>& x)
{
    bool y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y = ~(y | x[i]);
    }
    return y;
}

template<int32_t dim>
bool XOR(const glm::vec<dim, bool>& x)
{
    bool y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y ^= x[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline T CMIN(const glm::vec<dim, T>& x)
{
    T y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y = y < x[i] ? y : x[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline T CMAX(const glm::vec<dim, T>& x)
{
    T y = x[0];
    for(int32_t i = 1; i < dim; ++i)
    {
        y = y > x[i] ? y : x[i];
    }
    return y;
}

template<int32_t dim, typename T>
inline T DISTSQ(const glm::vec<dim, T>& a, const glm::vec<dim, T>& b)
{
    const glm::vec<dim, T> c = b - a;
    return glm::dot(c, c);
}

namespace AABB
{
    inline bool Intersects(
        const vec3& size_a, 
        const vec3& center_a,
        const vec3& size_b,
        const vec3& center_b)
    {
        return AND(
            GTE(
                center_a + size_a, 
                center_b - size_b)) 
            & 
            AND(
                LTE(
                    center_a - size_a, 
                    center_b + size_b));
    }
};
