#pragma once

#include <stdint.h>
#include <math.h>
#include "linmath.h"
#include <glm/gtc/matrix_transform.hpp>

#define UP vec3(0.0f, 1.0f, 0.0f)

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
        float yaw = 0.0f,
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
        P = glm::perspective(glm::radians(m_fov), m_whratio, m_near, m_far);
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
        m_eye += v.x * getRight(V) + v.y * getUp(V) - v.z * getForward(V);
    }
    void pitch(float amt)
    {
        m_pitch += amt;
        m_pitch = glm::max(glm::min(89.0f, m_pitch), -89.0f);
    }
    void yaw(float amt)
    {
        m_yaw -= amt;
        m_yaw = fmod(m_yaw, 360.0f);
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
