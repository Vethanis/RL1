#pragma once

#include <stdint.h>
#include "macro.h"
#include "strlcpy.h"
#include "store.h"
#include "hashstring.h"

struct Text
{
    char data[64];
};

struct Names
{
    Store<Text, 512> m_store;

    inline slot Create(const char* name)
    {
        slot s = m_store.Create(name);
        Text& t = m_store.GetUnchecked(s);
        strlcpy(t.data, name, NELEM(t.data));
        return s;
    }
    inline const char* operator[](slot s) const
    {
        const Text* text = m_store.Get(s);
        return text ? text->data : nullptr;
    }
    inline slot Find(Hash hash) const
    {
        return m_store.Find(hash);
    }
    inline slot Find(const char* name) const
    {
        return m_store.Find(name);
    }
    inline const char* Get(Hash hash) const
    {
        slot s = Find(hash);
        const Text* text = m_store.Get(s);
        return text ? text->data : nullptr;
    }

    static inline Names& GetSpace(Namespace ns)
    {
        return ms_names[ns];
    }

    static Names ms_names[NS_Count];
};
