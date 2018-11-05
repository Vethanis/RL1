#pragma once

#include <stdint.h>
#include "sokol_id.h"

struct ResMeta;

struct Image
{
    sg_image    id;
    uint8_t*    data;
    int32_t     width;
    int32_t     height;

    static void Load(const ResMeta& meta, Image& x);
    static void Free(const ResMeta& meta, Image& x);
    static void Init(const ResMeta& meta, Image& x);
    static void Shutdown(const ResMeta& meta, Image& x);
};
