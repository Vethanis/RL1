#pragma once

#include <stdint.h>

#include "slot.h"
#include "renderer.h"

namespace Images
{
    slot Create(const char* name);
    slot Create(const Renderer::TextureDesc& desc);
    void Destroy(slot s);
    const Renderer::Texture* Get(slot s);
};
