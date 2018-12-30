#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"

struct Image
{
    void*   data;
    int32_t width;
    int32_t height;
};

namespace Images
{
    slot Create(const char* name);
    slot Create(const Image& data);
    void Destroy(slot s);
    const sg_image* Get(slot s);
};
