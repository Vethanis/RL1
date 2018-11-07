#pragma once

#include <stdint.h>
#include "resource.h"
#include "sokol_id.h"

enum BufferType
{
    BT_File = 0,
    BT_Procedural,
    BT_Count
};

struct BufferResource : public Resource
{
    sg_buffer   m_id;
    int32_t     m_name;
    float*      m_verts;
    int32_t     m_count;
    BufferType  m_type;

    void Load() final;
    void Free() final;
    void Init() final;
    void Shutdown() final;

    static const ResourceType ms_type = RT_Buffer;
};
