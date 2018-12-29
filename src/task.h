#pragma once

#include <stdint.h>

enum TaskType
{
    TT_General = 0,
    TT_MeshGen,
    TT_Count
};

struct Task
{
    void (*fn)(Task*);
    uint64_t mem[4];
};

namespace TaskManager
{
    void Init();
    void Shutdown();
    void Start(TaskType type, int32_t granularity = 8);
    void Add(TaskType type, const Task& task);
};
