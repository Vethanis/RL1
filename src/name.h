#pragma once

#include <stdint.h>

#include "macro.h"
#include "array.h"
#include "fnv.h"

enum Namespace
{
    NS_Buffer = 0,
    NS_Image,
    NS_Count
};

struct Text
{
    char data[64];
};

struct Names
{
    Array<Text>     m_text;
    Array<uint64_t> m_hashes;

    int32_t Add(const char* name);
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

    static inline Names& Get(Namespace ns)
    {
        return ms_names[ns];
    }

    static Names ms_names[NS_Count];
};
