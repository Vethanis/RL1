#pragma once

#include "slot.h"
#include "sokol_gfx.h"

enum PipelineType
{
    PT_Textured = 0,
    PT_Sky,
    PT_Count,
};

namespace Pipelines
{
    void Create(PipelineType type, const void* desc);
    sg_pipeline Get(PipelineType type);
};
