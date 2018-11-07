#pragma once

enum ResourceType
{
    RT_None = 0,
    RT_Buffer,
    RT_Image,
    RT_Count
};

struct Resource
{
    virtual ~Resource(){};
    virtual void Load() = 0;        // async asset load (file io or proc gen)
    virtual void Free() = 0;        // free memory from load
    virtual void Init() = 0;        // create main thread graphics items
    virtual void Shutdown() = 0;    // destroy main thread graphics items
};

