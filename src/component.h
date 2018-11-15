#pragma once

#include <stdint.h>

#include "slot.h"
#include "array.h"
#include "transform.h"
#include "sokol_id.h"

enum ComponentType
{
    CT_Transform = 0,
    CT_Render,
    CT_Physics,
    CT_Children,
    CT_Pathfind,
    CT_AI,
    CT_Weapon,
    CT_Health,
    CT_Control,
    CT_Inventory,
    CT_Count  
};

struct Component {};

struct Row
{
    Component* m_components[CT_Count];

    template<typename T>
    inline const T* Get() const
    {
        return static_cast<T*>(m_components[T::ms_type]);
    }
};

namespace Components
{
    void Init();
    slot Create();
    void Destroy(slot s);
    const Row* Get(slot s);
    const Row& GetUnchecked(slot s);
    Component* Get(ComponentType type, slot s);
    const Component* GetConst(ComponentType type, slot s);
    void Add(ComponentType type, slot s);
    void Remove(ComponentType type, slot s);
    bool Exists(slot s);
    bool Has(ComponentType type, slot s);
    Component* GetAdd(ComponentType type, slot s);
    
    const slot* begin();
    const slot* end();

    template<typename T>
    inline T* Get(slot s)
    {
        return static_cast<T*>(Get(T::ms_type, s));
    }
    template<typename T>
    inline const T* GetConst(slot s)
    {
        return static_cast<T*>(GetConst(T::ms_type, s));
    }
    template<typename T>
    inline void Add(slot s)
    {
        Add(T::ms_type, s);
    }
    template<typename T>
    inline void Remove(slot s)
    {
        Remove(T::ms_type, s);
    }
    template<typename T>
    inline bool Has(slot s)
    {
        return Has(T::ms_type, s);
    }
    template<typename T>
    inline T* GetAdd(slot s)
    {
        return static_cast<T*>(GetAdd(T::ms_type, s));
    }
};

struct TransformComponent : public Component
{
    vec3        m_position;
    quat        m_rotation;
    vec3        m_scale;

    static const ComponentType ms_type = CT_Transform;
};

struct RenderComponent : public Component
{
    slot m_buf;
    slot m_img;
    slot m_pipeline;

    static const ComponentType ms_type = CT_Render;
};

struct PhysicsComponent : public Component
{
    void    (*m_handler)(slot, slot, vec3);
    vec3    m_hi;
    vec3    m_lo;
    vec3    m_moi;
    vec3    m_linearVelocity;
    quat    m_angularVelocity;
    vec3    m_linearDamping;
    quat    m_angularDamping;
    float   m_mass;

    void CalcMomentOfInertia()
    {
        vec3 dim = m_hi - m_lo;
        dim *= dim;
        m_moi = (m_mass / 12.0f) * vec3(
            dim.z + dim.y,
            dim.x + dim.z,
            dim.x + dim.y);
    }
    void Update(
        float       dt, 
        const vec3& linearForce, 
        const quat& angularForce,
        TransformComponent& xform)
    {
        const float inv = 1.0f / m_mass;
        vec3 linearAcc = linearForce * inv;
        quat angularAcc = angularForce * inv;

        m_linearVelocity += linearAcc * dt;
        m_angularVelocity *= angularAcc * dt;
        m_linearVelocity *= m_linearDamping;
        m_angularVelocity *= m_angularDamping;
        xform.m_position += m_linearVelocity * dt;
        xform.m_rotation *= m_angularVelocity * dt;
    }

    static const ComponentType ms_type = CT_Physics;
};

struct ChildrenComponent : public Component
{
    slot m_children[16];
    int32_t m_count;
    static const ComponentType ms_type = CT_Children;
};

struct PathfindComponent : public Component
{
    vec3 m_goal;
    vec3 m_path[16];
    int32_t m_count;
    static const ComponentType ms_type = CT_Pathfind;
};

struct AIComponent : public Component
{
    static const ComponentType ms_type = CT_AI;
};

struct WeaponComponent : public Component
{
    float m_damage;
    float m_rate;
    float m_range;
    static const ComponentType ms_type = CT_Weapon;
};

struct HealthComponent : public Component
{
    float m_health;
    static const ComponentType ms_type = CT_Health;
};

struct ControlComponent : public Component
{
    float m_axis[8];
    bool  m_buttons[32];
    static const ComponentType ms_type = CT_Control;
};

struct InventoryComponent : public Component
{
    slot m_items[64];
    int32_t m_count;
    static const ComponentType ms_type = CT_Inventory;
};
