#pragma once

#include <stdint.h>
#include "comptype.h"
#include "sokol_id.h"

struct LoaderMeta;

struct Image
{
    sg_image    id;
    uint8_t*    data;
    int32_t     width;
    int32_t     height;

    static void Load(const LoaderMeta& meta, Image& x);
    static void Free(const LoaderMeta& meta, Image& x);
    static void Init(const LoaderMeta& meta, Image& x);
    static void Shutdown(const LoaderMeta& meta, Image& x);

    static const ComponentType ms_type = CT_Image;
};
