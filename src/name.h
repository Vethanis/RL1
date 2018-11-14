#pragma once

#include <stdint.h>
#include "macro.h"
#include "strlcpy.h"
#include "store.h"

enum Namespace
{
    NS_Buffer = 0,
    NS_Image,
    NS_Entity,
    NS_Count
};

struct Text
{
    char data[64];
};

struct Names
{
    Store<Text, 64> m_store;

    inline slot Create(const char* name)
    {
        slot s = m_store.Create(name);
        Text* t = m_store.Get(s);
        strlcpy(t->data, name, NELEM(t->data));
    }
    inline void Destroy(slot s)
    {
        m_store.Destroy(s);
    }
    inline const char* operator[](slot s) const
    {
        const Text* text = m_store.Get(s);
        return text ? text->data : nullptr;
    }
    inline slot Find(uint64_t hash) const
    {
        return m_store.Find(hash);
    }
    inline slot Find(const char* name) const
    {
        return m_store.Find(name);
    }

    static inline Names& Get(Namespace ns)
    {
        return ms_names[ns];
    }

    static Names ms_names[NS_Count];
};
