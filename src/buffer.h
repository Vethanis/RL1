#pragma once

#include <stdint.h>
#include "sokol_id.h"

enum BufferLoader
{
    BL_File = 0,
    BL_Gen,
    BL_Count
};

struct ResMeta;

struct Buffer
{
    sg_buffer   id;
    float*      verts;
    int32_t     count;

    static void Load(const ResMeta& meta, Buffer& x);
    static void Free(const ResMeta& meta, Buffer& x);
    static void Init(const ResMeta& meta, Buffer& x);
    static void Shutdown(const ResMeta& meta, Buffer& x);
};
