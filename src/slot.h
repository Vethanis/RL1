#pragma once

#include <stdint.h>

union slot
{
    struct
    {
        uint16_t id;
        uint16_t gen;
    };
    uint32_t value;

    inline slot()
    {
        value = 0xFFFFFFFF;
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
