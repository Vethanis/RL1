#pragma once

#include <stdint.h>
#include "resource.h"

struct Task
{
    void (*fn)(Task*);
    uint64_t mem[3];
};

namespace TaskManager
{
    // will use up to ms milliseconds to do parallel updates; can early out
    // begins multithreaded phase of engine frame; blocking
    void Start(ResourceType type, uint64_t ms);
    // add task to task stack (not a queue!)
    void Add(ResourceType type, const Task& task);
};
