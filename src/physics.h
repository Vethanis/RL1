#pragma once

#include "macro.h"
#include "slot.h"
#include "linmath.h"
#include "component.h"

#include <bullet3/btBulletDynamicsCommon.h>

inline btVector3 toB3(const vec3& x)
{
    return btVector3(x.x, x.y, x.z);
}

inline vec3 toGLM(const btVector3& x)
{
    return vec3(x.x(), x.y(), x.z());
}

inline btTransform toB3(const mat4& x)
{
    btTransform y;
    y.setFromOpenGLMatrix(glm::value_ptr(x));
    return y;
}

inline mat4 toGLM(const btTransform& x)
{
    mat4 y;
    x.getOpenGLMatrix(glm::value_ptr(y));
    return y;
}

namespace Physics
{
    void Init(float timestep);
    void Update(float dt);
    btRigidBody* Create(float mass, const vec3& position, const vec3& extent);
    void Destroy(btRigidBody* body);
};

struct PhysicsComponent
{
    btRigidBody* m_body;

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
        m_body->setWorldTransform(toB3(xform));
    }
    inline mat4 GetTransform() const 
    {
        const btTransform& xform = m_body->getWorldTransform();
        return toGLM(xform);
    }

    static const ComponentType ms_type = CT_Physics;
};
