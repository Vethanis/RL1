#pragma once

#include <stdint.h>

#include "slot.h"
#include "array.h"
#include "transform.h"

enum ComponentType
{
    CT_Transform = 0,
    CT_Buffer,
    CT_Image,
    CT_Material,
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

struct TransformComponent : public Component
{
    Transform m_mat;
    static const ComponentType ms_type = CT_Transform;
};

struct BufferComponent : public Component
{
    slot m_ref;
    static const ComponentType ms_type = CT_Buffer;
};

struct ImageComponent : public Component
{
    slot m_ref;
    static const ComponentType ms_type = CT_Image;
};

struct MaterialComponent : public Component
{
    vec2  m_texScale;
    float m_metalness;
    float m_roughness;
    float m_alpha;
    float m_brightness;
    static const ComponentType ms_type = CT_Material;
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
    vec3                m_goal;
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
