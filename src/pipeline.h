#pragma once

#include "slot.h"
#include "sokol_gfx.h"

namespace Pipelines
{
    slot Create(const sg_pipeline_desc& desc);
    void Destroy(slot s);
    const sg_pipeline* Get(slot s);
    bool Exists(slot s);
};
