#pragma once

#include <stdint.h>

enum TaskType
{
    TT_General = 0,
    TT_Count
};

struct Task
{
    void (*fn)(Task*);
    uint64_t mem[3];
};

namespace TaskManager
{
    void Init();
    void Shutdown();
    // will use up to ms milliseconds to do parallel updates; can early out
    // begins multithreaded phase of engine frame; blocking
    void Start(TaskType type, uint64_t ms);
    // add task to task stack (not a queue!)
    void Add(TaskType type, const Task& task);
};
