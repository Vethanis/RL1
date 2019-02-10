#pragma once

#include <stdint.h>
#include <math.h>
#include "linmath.h"

struct Camera
{
    mat4    P;
    mat4    V;
    vec3    m_eye;
    float   m_fov;
    vec3    m_at;
    float   m_whratio;
    float   m_near;
    float   m_far;
    float   m_yaw;
    float   m_pitch;

    inline void Init(
        int32_t width,
        int32_t height, 
        float yaw = -90.0f,
        float pitch = 0.0f,
        float fov=60.0f, 
        float near=0.1f, 
        float far=100.0f)
    {
        P = mat4(1.0f);
        V = mat4(1.0f);
        m_eye = vec3(0.0f);
        m_at = vec3(0.0f, 0.0f, -1.0f);
        m_fov = fov;
        m_whratio = (float)width / (float)height;
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
    inline mat4 update()
    {
        updateP();
        updateV();
        return P * V;
    }
    inline void resize(int32_t width, int32_t height)
    {
        m_whratio = (float)width / (float)height;
    }
    inline void move(const vec3& v)
    {
        m_eye += 
            v.x * getRight(V) + 
            v.y * getUp(V) + 
            v.z * getForward(V);
    }
    inline void pitch(float amt)
    {
        m_pitch = glm::clamp(m_pitch + amt, -89.0f, 89.0f);
    }
    inline void yaw(float amt)
    {
        m_yaw = fmod(m_yaw - amt, 360.0f);
    }
    inline vec3 direction() const { return getForward(V); }

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

