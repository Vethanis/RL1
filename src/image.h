#pragma once

#include <stdint.h>

#include "slot.h"
#include "sokol_id.h"
#include "hashstring.h"

struct Image
{
    void*   data;
    int32_t width;
    int32_t height;
};

namespace Images
{
    slot Load(ImageString name);
    void Destroy(slot s);
    void IncRef(slot s);
    void DecRef(slot s);
    const sg_image* Get(slot s);
    bool Exists(slot s);
    bool Exists(const char* name);
    slot Find(const char* name);
    slot Find(Hash hash);

    Image Load(const char* name);
    void Free(Image& img);
    sg_image Create(const Image& img);
    void Destroy(sg_image img);
};
