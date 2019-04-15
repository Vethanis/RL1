#pragma once

#include <math.h>
#include "hlsl_types.h"

struct Camera
{
    float4x4    P;
    float4x4    V;
    float4      m_eye;
    f32         m_fov;
    float4      m_at;
    f32         m_whratio;
    f32         m_near;
    f32         m_far;
    f32         m_yaw;
    f32         m_pitch;

    inline void Init(
        i32 width,
        i32 height, 
        f32 yaw = -90.0f,
        f32 pitch = 0.0f,
        f32 fov=60.0f, 
        f32 near=0.1f, 
        f32 far=100.0f)
    {
        P = float4x4(1.0f);
        V = float4x4(1.0f);
        m_eye = float4(0.0f);
        m_at = float4(0.0f, 0.0f, -1.0f);
        m_fov = fov;
        m_whratio = (f32)width / (f32)height;
        m_near = near;
        m_far = far;
        m_yaw = yaw;
        m_pitch = pitch;
        update();
    }
    inline void updateP()
    {
        P = glm::perspective(
            glm::radians(m_fov), 
            m_whratio, 
            m_near, 
            m_far);
    }
    inline void updateV()
    {
        m_at.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
        m_at.y = sinf(glm::radians(m_pitch));
        m_at.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
        m_at += m_eye;
        V = glm::lookAt(m_eye, m_at, UP);
    }
    inline float4x4 update()
    {
        updateP();
        updateV();
        return P * V;
    }
    inline void resize(i32 width, i32 height)
    {
        m_whratio = (f32)width / (f32)height;
    }
    inline void move(const float4& v)
    {
        m_eye += 
            v.x * getRight(V) + 
            v.y * getUp(V) + 
            v.z * getForward(V);
    }
    inline void pitch(f32 amt)
    {
        m_pitch = glm::clamp(m_pitch + amt, -89.0f, 89.0f);
    }
    inline void yaw(f32 amt)
    {
        m_yaw = fmod(m_yaw - amt, 360.0f);
    }
    inline float4 direction() const { return getForward(V); }

    static Camera* GetActive();
    static void SetActive(Camera* cam);
};

struct PushCamera
{
    Camera* prev;

    inline PushCamera(Camera* toPush)
    {
        prev = Camera::GetActive();
        Camera::SetActive(toPush);
    }
    ~PushCamera()
    {
        Camera::SetActive(prev);
    }
};
