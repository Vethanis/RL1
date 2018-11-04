#pragma once

#include <stdint.h>

#include "macro.h"
#include "array.h"
#include "fnv.h"

struct Text
{
    char data[64];
};

// use a separate one of these for each resource for more speed
struct Names
{
    Array<Text>     m_text;
    Array<uint64_t> m_hashes;

    int32_t Add(const char* name)
    {
        Text& t = m_text.grow();
        uint64_t hash = m_hashes.grow();
        hash = Fnv64(name);

        int32_t i = 0;
        const int32_t len = NELEM(t.data) - 1;
        for(; i < len && name[i]; ++i)
        {
            t.data[i] = name[i];
        }
        t.data[i] = '\0';

        return m_text.count() - 1;
    }
    inline const char* operator[](int32_t i) const
    {
        return m_text[i].data;
    }
    inline int32_t Find(uint64_t hash) const
    {
        return m_hashes.find(hash);
    }
    inline int32_t Find(const char* name) const
    {
        return Find(Fnv64(name));
    }
};
