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
    CT_Collision,
    CT_Rigidbody,
    CT_Children,
    CT_Pathfind,
    CT_AI,
    CT_Weapon,
    CT_Health,
    CT_Control,
    CT_Inventory,
    CT_Count  
};

struct Component 
{
    virtual ~Component(){}
};

namespace Components
{
    void Init();
    slot Create(const char* name);
    void Destroy(slot s);
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
    Transform m_mat;

    static const ComponentType ms_type = CT_Transform;
};

struct RenderComponent : public Component
{
    slot m_buf;
    slot m_img;
    slot m_pipeline;

    static const ComponentType ms_type = CT_Render;
};

struct CollisionComponent : public Component
{
    void (*m_handler)(slot, slot);
    vec3 m_aabb[2];
    static const ComponentType ms_type = CT_Collision;
};

struct RigidbodyComponent : public Component
{
    float m_mass;
    vec3  m_linearDamping;
    vec3  m_angularDamping;
    vec3  m_linearVelocity;
    vec3  m_angularVelocity;
    static const ComponentType ms_type = CT_Rigidbody;
};

struct ChildrenComponent : public Component
{
    Array<slot> m_children;
    static const ComponentType ms_type = CT_Children;
};

struct PathfindComponent : public Component
{
    Array<vec3>   m_path;
    vec3          m_goal;
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
    Array<slot> m_items;
    static const ComponentType ms_type = CT_Inventory;
};
