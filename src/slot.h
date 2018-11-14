#pragma once

#include <stdint.h>

struct slot
{
    uint32_t id;
    uint32_t gen;

    inline bool operator == (slot other) const 
    {
        return id == other.id && gen == other.gen;
    }
    inline bool operator != (slot other) const 
    {
        return id != other.id || gen != other.gen;
    }
    static inline slot CreateInvalid()
    {
        return { 0xFFFFFFFF, 0xFFFFFFFF };
    }
    static inline bool IsInvalid(slot s)
    {
        return s == CreateInvalid();
    }
};
