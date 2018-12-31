#pragma once

#include <stdint.h>
#include "slot.h"
#include "renderer.h"

namespace Buffers
{
    slot Create(const Renderer::BufferDesc& desc);
    void Destroy(slot s);
    const Renderer::Buffer* Get(slot s);
};
