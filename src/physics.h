#pragma once

#include "macro.h"
#include "slot.h"
#include "linmath.h"
#include "component.h"

namespace Physics
{
    void Init();
    void Update(float dt);
    void Shutdown();
    void* Create(float mass, const vec3& position, const vec3& extent);
    void Destroy(void* body);
};

struct PhysicsComponent
{
    void* m_body;

    inline void Init(float mass, const vec3& position, const vec3& size)
    {
        m_body = Physics::Create(mass, position, size);
    }
    inline void Shutdown()
    {
        Physics::Destroy(m_body);
        m_body = nullptr;
    }
    inline void SetTransform(const mat4& xform)
    {
        
    }
    inline mat4 GetTransform() const 
    {
        return mat4(1.0f);
    }

    static const ComponentType ms_type = CT_Physics;
};
