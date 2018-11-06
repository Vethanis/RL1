#pragma once

#include <stdint.h>
#include "slot.h"
#include "comptype.h"
#include "sokol_id.h"

enum BufferLoader
{
    BL_File = 0,
    BL_Gen,
    BL_Count
};

struct LoaderMeta;

struct Buffer
{
    sg_buffer   id;
    float*      verts;
    int32_t     count;

    static void Load(const LoaderMeta& meta, Buffer& x);
    static void Free(const LoaderMeta& meta, Buffer& x);
    static void Init(const LoaderMeta& meta, Buffer& x);
    static void Shutdown(const LoaderMeta& meta, Buffer& x);

    static const ComponentType ms_type = CT_Buffer;
};
