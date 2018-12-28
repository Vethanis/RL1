#pragma once

#include "slot.h"
#include "sokol_gfx.h"

enum PipelineType
{
    PT_Textured = 0,
    PT_Count,
};

namespace Pipelines
{
    void Create(PipelineType type, const sg_pipeline_desc& desc);
    sg_pipeline Get(PipelineType type);
};
