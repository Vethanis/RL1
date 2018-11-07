#pragma once

#include <stdint.h>
#include "resource.h"
#include "sokol_id.h"

enum ImageType
{
    IT_File = 0,
    IT_Procedural,
    IT_Count
};

struct ImageResource : public Resource
{
    sg_image    m_id;
    int32_t     m_name;
    uint8_t*    m_data;
    int32_t     m_width;
    int32_t     m_height;
    ImageType   m_type;

    void Load() final;
    void Free() final;
    void Init() final;
    void Shutdown() final;

    static const ResourceType ms_type = RT_Image;
};
