#pragma once

#include "linmath.h"
#include "array.h"

struct GLShader
{
    struct Uniform
    {
        int32_t  location;
        uint64_t hash;

        inline bool operator==(const Uniform& other) const 
        {
            return hash == other.hash;
        }
    };

    uint32_t                m_id;
    FixedArray<Uniform, 32> m_uniforms;

    void Init(const char* vs, const char* fs);
    void Shutdown();
    void Use();
    void SetInt(const char* name, int32_t value);
    void SetFloat(const char* name, float value);
    void SetVec2(const char* name, const vec2& value);
    void SetVec3(const char* name, const vec3& value);
    void SetVec4(const char* name, const vec4& value);
    void SetMat3(const char* name, const mat3& value);
    void SetMat4(const char* name, const mat4& value);
    int32_t GetUniformLocation(const char* name);

    static GLShader* ms_current;
};
