#pragma once

#include "slots.h"
#include "resource.h"

template<typename T>
struct Resource : public ResourceBase
{
    Slots<T> m_slots;
    inline void Add(slot s)     final { m_slots.Add(s); };
    inline void Remove(slot s)  final { m_slots.Remove(s); };
    inline bool Exists(slot s)  final { return m_slots.Exists(s); };
    inline void* Get(slot s)    final { return (void*)m_slots.Get(s); };
};
