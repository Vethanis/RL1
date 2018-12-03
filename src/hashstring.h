#pragma once

#include <stdint.h>
#include "fnv.h"
#include "slot.h"

enum Namespace
{
    NS_Default = 0,
    NS_Pipeline,
    NS_Shader,
    NS_Buffer,
    NS_Image,
    NS_Model,
    NS_Entity,
    NS_Count
};

// doesnt save anything, just for lookup
struct Hash
{
    uint64_t value;

    inline Hash()
    {
        value = 0;
    }
    inline explicit Hash(uint64_t x)
    {
        value = x;
    }
    inline explicit Hash(const char* x)
    {
        value = Fnv64(x);
        value = value ? value : 1;
    }
    inline bool IsNull() const 
    {
        return value == 0;
    }
    inline bool IsValid() const 
    {
        return value != 0;
    }
    inline bool operator==(Hash other) const 
    {
        return value == other.value;
    }
    inline bool operator!=(Hash other) const 
    {
        return value != other.value;
    }
    inline operator uint64_t () const
    {
        return value;
    }
};

slot SlotStringConstructor(Namespace ns, const char* x);
slot SlotStringLookup(Namespace ns, Hash hash);
const char* SlotStringGetString(Namespace ns, slot s);

// saves the embedded string *somewhere*
template<Namespace space = NS_Default>
struct SlotString
{
    slot m_slot;

    inline SlotString(){}
    inline explicit SlotString(slot x)
    {
        m_slot = x;
    }
    inline explicit SlotString(Hash x)
    {
        m_slot = SlotStringLookup(space, x);
    }
    inline explicit SlotString(const char* x)
    {
        m_slot = SlotStringConstructor(space, x);
    }
    inline const char* GetStr() const
    {
        return SlotStringGetString(space, m_slot);
    }
    inline bool operator==(SlotString other) const 
    {
        return m_slot == other.m_slot;
    }
    inline bool operator!=(SlotString other) const 
    {
        return m_slot != other.m_slot;
    }
    inline bool IsNull() const 
    {
        return m_slot == slot();
    }
    inline bool IsValid() const 
    {
        return m_slot != slot();
    }
};

typedef SlotString<NS_Default>  DefaultString;
typedef SlotString<NS_Pipeline> PipelineString;
typedef SlotString<NS_Shader>   ShaderString;
typedef SlotString<NS_Buffer>   BufferString;
typedef SlotString<NS_Image>    ImageString;
typedef SlotString<NS_Model>    ModelString;
typedef SlotString<NS_Entity>   EntityString;
