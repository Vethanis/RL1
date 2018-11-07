#include "row.h"

#include "array.h"
#include "macro.h"
#include "compalloc.h"
#include "blockalloc.h"

#include "transform.h"
#include "buffer.h"
#include "image.h"

namespace Rows
{
    Array<Row>               ms_rows;
    Array<uint32_t>          ms_free;
    ComponentAllocator*      ms_allocators[CT_Count] = {0};

    void Init()
    {
        if(ms_allocators[0])
        {
            return;
        }
        ms_allocators[CT_Transform] = new BlockAlloc<TransformComponent>();
        ms_allocators[CT_Buffer] = new BlockAlloc<BufferComponent>();
        ms_allocators[CT_Image] = new BlockAlloc<ImageComponent>();
        ms_allocators[CT_Material] = new BlockAlloc<MaterialComponent>();
        ms_allocators[CT_Collision] = new BlockAlloc<CollisionComponent>();
        ms_allocators[CT_Rigidbody] = new BlockAlloc<RigidbodyComponent>();
        ms_allocators[CT_Children] = new BlockAlloc<ChildrenComponent>();
        ms_allocators[CT_Pathfind] = new BlockAlloc<PathfindComponent>();
        ms_allocators[CT_AI] = new BlockAlloc<AIComponent>();
        ms_allocators[CT_Weapon] = new BlockAlloc<WeaponComponent>();
        ms_allocators[CT_Health] = new BlockAlloc<HealthComponent>();
        ms_allocators[CT_Control] = new BlockAlloc<ControlComponent>();
        ms_allocators[CT_Inventory] = new BlockAlloc<InventoryComponent>();
    }
    slot Create()
    {
        if(ms_free.empty())
        {
            Row row;
            memset(&row, 0, sizeof(Row));
            ms_free.grow() = (uint32_t)ms_rows.count();
            ms_rows.grow() = row;
        }
        slot s;
        s.id = ms_free.back();
        ms_free.pop();
        s.gen = ms_rows[s.id].m_gen;
        return s;
    }
    void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        Row& row = ms_rows[s.id];
        for(int32_t i = 0; i < CT_Count; ++i)
        {
            if(row.m_components[i])
            {
                ms_allocators[i]->Free(row.m_components[i]);
                row.m_components[i] = nullptr;
            }
        }
        row.m_gen++;
        ms_free.grow() = s.id;
    }
    Component* Get(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Row& row = ms_rows[s.id];
        return row.m_components[type];
    }
    void Add(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        Row& row = ms_rows[s.id];
        Component* c = row.m_components[type];
        if(!c)
        {
            row.m_components[type] = ms_allocators[type]->Alloc();
        }
    }
    void Remove(ComponentType type, slot s)
    {
        if(!Has(type, s))
        {
            return;
        }
        Row& row = ms_rows[s.id];
        ms_allocators[type]->Free(row.m_components[type]);
        row.m_components[type] = nullptr;
    }
    bool Exists(slot s) 
    {
        return s.id < (uint32_t)ms_rows.count() && s.gen == ms_rows[s.id].m_gen;
    }
    bool Has(ComponentType type, slot s)
    {
        return Exists(s) && (ms_rows[s.id].m_components[type] != nullptr);
    }
    Component* GetAdd(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Row& row = ms_rows[s.id];
        Component* c = row.m_components[type];
        if(!c)
        {
            c = ms_allocators[type]->Alloc();
            row.m_components[type] = c;
        }
        return c;
    }
};
