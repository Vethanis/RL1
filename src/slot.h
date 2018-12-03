#pragma once

#include <stdint.h>

union slot
{
    struct
    {
        uint32_t id;
        uint32_t gen;
    };
    uint64_t value;

    inline slot()
    {
        value = 0xFFFFFFFFFFFFFFFF;
    }
    inline bool operator == (slot other) const 
    {
        return value == other.value;
    }
    inline bool operator != (slot other) const 
    {
        return value != other.value;
    }
    static inline bool IsInvalid(slot s)
    {
        return s == slot();
    }
};
