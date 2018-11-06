#pragma once

#include <stdint.h>
#include "slot.h"
#include "comptype.h"

struct TaskManager;

namespace Component
{
    bool HasLoader(ComponentType type);
    void Add(ComponentType type, slot s);
    void Add(ComponentType type, slot s, int32_t name, uint8_t loader);
    void Remove(ComponentType type, slot s);

    template<typename T>
    T* Get(slot s);

    bool Exists(ComponentType type, slot s);
    void Update(ComponentType type);
    void DoTasks(ComponentType type);

    void UpdateAll();
    void RemoveAll(slot s);
};
