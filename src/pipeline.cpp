#include "pipeline.h"
#include "macro.h"

namespace Pipelines
{
    sg_pipeline pipelines[PT_Count];

    void Create(PipelineType type, const sg_pipeline_desc& desc)
    {
        pipelines[type] = sg_make_pipeline(&desc);
        Assert(pipelines[type].id != SG_INVALID_ID);
    }
    sg_pipeline Get(PipelineType type)
    {
        return pipelines[type];
    }
};
