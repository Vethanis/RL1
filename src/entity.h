#pragma once

#include <stdint.h>

#include "array.h"
#include "slots.h"
#include "macro.h"

#include "resourcebase.h"

typedef slot Entity;

struct Entities
{
    Array<slot>     m_slots;
    Array<int32_t>  m_free;
    Array<int32_t>  m_live;

    Entity Create()
    {
        slot s = {0};
        if(!m_free.empty())
        {
            s.id = m_free.back();
            s.gen = m_slots[s.id].gen;
            m_free.pop();
        }
        else
        {
            s.id = m_slots.count() - 1;
            s.gen = 0u;
            m_slots.grow() = s;
        }

        m_live.grow() = s.id;

        return s;
    }
    void Destroy(Entity s)
    {
        Assert(s.id >= 0 && s.id < m_slots.count());
        if(s.gen != m_slots[s.id].gen)
        {
            return;
        }

        ResourceBase::RemoveAll(s);

        m_slots[s.id].gen++;
        m_free.grow() = s.id;
        m_live.findRemove(s.id);
    }
    inline int32_t Count() const { return m_live.count(); }
    inline Entity operator[](int32_t i) const
    {
        int32_t idx = m_live[i];
        return m_slots[idx];
    }
};
