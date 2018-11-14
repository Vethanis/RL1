#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"

namespace Images
{
    slot Create(const char* name);
    slot Create(const char* name, const void* data, int32_t width, int32_t height);
    void Destroy(slot s);
    const sg_image* Get(slot s);
    bool Exists(slot s);
    bool Exists(const char* name);
    slot Find(const char* name);
    slot Find(uint64_t hash);
};
