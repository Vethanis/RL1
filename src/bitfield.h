#pragma once

#include "macro.h"
#include "scalar_types.h"
#include "slice.h"

template<u32 bitCount>
struct BitField
{
    static constexpr u32 sc_DwordCount = DivCeil(bitCount, 32u);

    u32 m_dwords[sc_DwordCount];

    inline bool has(u32 bit) const
    {
        DebugAssert(bit < bitCount);
        const u32 idx       = bit >> 5u;
        const u32 dwordBit  = bit &  31u;
        const u32 dword     = m_dwords[idx];
        const bool bitValue = (dword >> dwordBit) & 1u;
        return bitValue;
    }

    inline void set(u32 bit, bool isSet)
    {
        DebugAssert(bit < bitCount);
        const u32 idx       = bit >> 5u;
        const u32 dwordBit  = bit & 31u;
        const u32 mask      = 1u << dwordBit;
        const u32 dword     = m_dwords[idx];
        const u32 asSet     = dword | mask;
        const u32 asUnset   = dword & (~mask);
        m_dwords[idx]       = isSet ? asSet : asUnset;
    }
    inline void set(u32 bit)
    {
        DebugAssert(bit < bitCount);
        const u32 idx       = bit >> 5u;
        const u32 dwordBit  = bit & 31u;
        const u32 mask      = 1u << dwordBit;
        m_dwords[idx]      |= mask;
    }
    inline void set(CDwordSlice bits)
    {
        for(u32 bit : bits)
        {
            set(bit);
        }
    }

    inline void unset(u32 bit)
    {
        DebugAssert(bit < bitCount);
        const u32 idx       = bit >> 5u;
        const u32 dwordBit  = bit & 31u;
        const u32 mask      = 1u << dwordBit;
        m_dwords[idx]       &= ~mask;
    }
    
    inline void toggle(u32 bit)
    {
        const u32 idx       = bit >> 5u;
        const u32 dwordBit  = bit & 31u;
        const u32 mask      = 1u << dwordBit;
        m_dwords[idx]       ^= mask;
    }

    inline bool has_all(const BitField& other) const
    {
        const u32* has   = m_dwords;
        const u32* wants = other.m_dwords;
        for(u32 i = 0u; i < sc_DwordCount; ++i)
        {
            if((has[i] & wants[i]) != wants[i])
            {
                return false;
            }
        }
        return true;
    }
};
