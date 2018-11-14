#include "component.h"

#include "store.h"
#include "macro.h"
#include "compalloc.h"
#include "blockalloc.h"

#include "transform.h"
#include "buffer.h"
#include "image.h"

namespace Components
{
    struct Row
    {
        Component*  m_components[CT_Count];
    };
    
    Store<Row, 256>     ms_rows;
    Array<slot>         ms_alive;
    ComponentAllocator* ms_allocs[CT_Count] = {0};

    void Init()
    {
        if(ms_allocs[0])
        {
            return;
        }
        ms_allocs[CT_Transform] = new BlockAlloc<TransformComponent>();
        ms_allocs[CT_Render] = new BlockAlloc<RenderComponent>();
        ms_allocs[CT_Collision] = new BlockAlloc<CollisionComponent>();
        ms_allocs[CT_Rigidbody] = new BlockAlloc<RigidbodyComponent>();
        ms_allocs[CT_Children] = new BlockAlloc<ChildrenComponent>();
        ms_allocs[CT_Pathfind] = new BlockAlloc<PathfindComponent>();
        ms_allocs[CT_AI] = new BlockAlloc<AIComponent>();
        ms_allocs[CT_Weapon] = new BlockAlloc<WeaponComponent>();
        ms_allocs[CT_Health] = new BlockAlloc<HealthComponent>();
        ms_allocs[CT_Control] = new BlockAlloc<ControlComponent>();
        ms_allocs[CT_Inventory] = new BlockAlloc<InventoryComponent>();
    }
    slot Create(const char* name)
    {
        slot s = ms_rows.Create(name);
        ms_alive.grow() = s;
        return s;
    }
    void Destroy(slot s)
    {
        if(ms_rows.Exists(s))
        {
            Row& row = ms_rows.GetUnchecked(s);
            for(int32_t i = 0; i < CT_Count; ++i)
            {
                if(row.m_components[i])
                {
                    ms_allocs[i]->Free(row.m_components[i]);
                    row.m_components[i] = nullptr;
                }
            }
            ms_rows.DestroyUnchecked(s);
            ms_alive.findRemove(s);
        }
    }
    Component* Get(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Row& row = ms_rows.GetUnchecked(s);
        return row.m_components[type];
    }
    const Component* GetConst(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        const Row& row = ms_rows.GetUnchecked(s);
        return row.m_components[type];
    }
    void Add(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        Row& row = ms_rows.GetUnchecked(s);
        Component* c = row.m_components[type];
        if(!c)
        {
            row.m_components[type] = ms_allocs[type]->Alloc();
        }
    }
    void Remove(ComponentType type, slot s)
    {
        if(!Has(type, s))
        {
            return;
        }
        Row& row = ms_rows.GetUnchecked(s);
        ms_allocs[type]->Free(row.m_components[type]);
        row.m_components[type] = nullptr;
    }
    bool Exists(slot s) 
    {
        return ms_rows.Exists(s);
    }
    bool Has(ComponentType type, slot s)
    {
        return Exists(s) && (ms_rows.GetUnchecked(s).m_components[type] != nullptr);
    }
    Component* GetAdd(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Row& row = ms_rows.GetUnchecked(s);
        Component* c = row.m_components[type];
        if(!c)
        {
            c = ms_allocs[type]->Alloc();
            row.m_components[type] = c;
        }
        return c;
    }
    const slot* begin()
    {
        return ms_alive.begin();
    }
    const slot* end()
    {
        return ms_alive.end();
    }
};
