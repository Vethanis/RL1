#include "component.h"

#include "gen_array.h"
#include "macro.h"
#include "blockalloc.h"

#include "rendercomponent.h"
#include "physics.h"

struct Row
{
    void* m_components[CT_Count];

    template<typename T>
    inline const T* Get() const
    {
        return static_cast<const T*>(m_components[T::ms_type]);
    }
    template<typename T>
    inline T* Get()
    {
        return static_cast<T*>(m_components[T::ms_type]);
    }
};

namespace Components
{
    gen_array<Row>      ms_rows;
    Array<slot>         ms_alive;
    BlockAlloc          ms_allocs[CT_Count];
    bool                ms_hasInit = false;

    void Init()
    {
        if(ms_hasInit)
        {
            return;
        }
        ms_hasInit = true;
        BucketScope scope(AB_Default);
        ms_allocs[CT_Render].Init<RenderComponent>();
        ms_allocs[CT_Physics].Init<PhysicsComponent>();
    }
    slot Create()
    {
        BucketScope scope(AB_Default);
        slot s = ms_rows.Create();
        ms_alive.grow() = s;
        return s;
    }
    void CleanupPhysics(Row& row)
    {
        PhysicsComponent* pc = row.Get<PhysicsComponent>();
        if(pc)
        {
            pc->Shutdown();
        }
    }
    void Destroy(slot s)
    {
        if(ms_rows.Exists(s))
        {
            BucketScope scope(AB_Default);

            Row& row = ms_rows.GetUnchecked(s);

            CleanupPhysics(row);

            for(int32_t i = 0; i < CT_Count; ++i)
            {
                if(row.m_components[i])
                {
                    ms_allocs[i].Free(row.m_components[i]);
                    row.m_components[i] = nullptr;
                }
            }

            ms_rows.DestroyUnchecked(s);
            ms_alive.findRemove(s);
        }
    }
    void* Get(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        Row& row = ms_rows.GetUnchecked(s);
        return row.m_components[type];
    }
    const void* GetConst(ComponentType type, slot s)
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
        BucketScope scope(AB_Default);
        Row& row = ms_rows.GetUnchecked(s);
        void* c = row.m_components[type];
        if(!c)
        {
            row.m_components[type] = ms_allocs[type].Alloc();
        }
    }
    void Remove(ComponentType type, slot s)
    {
        if(!Has(type, s))
        {
            return;
        }
        BucketScope scope(AB_Default);
        Row& row = ms_rows.GetUnchecked(s);

        if(type == CT_Physics)
        {
            CleanupPhysics(row);
        }

        ms_allocs[type].Free(row.m_components[type]);
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
    void* GetAdd(ComponentType type, slot s)
    {
        if(!Exists(s))
        {
            return nullptr;
        }
        BucketScope scope(AB_Default);
        Row& row = ms_rows.GetUnchecked(s);
        void* c = row.m_components[type];
        if(!c)
        {
            c = ms_allocs[type].Alloc();
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
