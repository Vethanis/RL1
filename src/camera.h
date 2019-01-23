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

    void Init(
        int width,
        int height, 
        float yaw = -90.0f,
        float pitch = 0.0f,
        float fov=60.0f, 
        float near=0.1f, 
        float far=100.0f)
    {
        P = mat4();
        V = mat4();
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
    void updateP()
    {
        P = glm::perspective(
            glm::radians(m_fov), 
            m_whratio, 
            m_near, 
            m_far);
    }
    void updateV()
    {
        m_at.x = cosf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
        m_at.y = sinf(glm::radians(m_pitch));
        m_at.z = sinf(glm::radians(m_yaw)) * cosf(glm::radians(m_pitch));
        m_at += m_eye;
        V = glm::lookAt(m_eye, m_at, UP);
    }
    mat4 update()
    {
        updateP();
        updateV();
        return P * V;
    }
    void resize(int32_t width, int32_t height)
    {
        m_whratio = (float)width / (float)height;
    }
    void setFov(float fov)
    {
        m_fov = fov;
    }
    void move(const vec3& v)
    {
        m_eye += 
            v.x * getRight(V) + 
            v.y * getUp(V) + 
            v.z * getForward(V);
    }
    void pitch(float amt)
    {
        m_pitch = glm::clamp(m_pitch + amt, -89.0f, 89.0f);
    }
    void yaw(float amt)
    {
        m_yaw = fmod(m_yaw - amt, 360.0f);
    }
    const vec3 direction() const { return normalize(m_at - m_eye); }
    void setPlanes(float near, float far)
    {
        m_near = near; 
        m_far = far;
    }

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

