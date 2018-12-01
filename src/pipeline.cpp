#include "pipeline.h"

#include "macro.h"
#include "gen_array.h"

namespace Pipelines
{
    gen_array<sg_pipeline> ms_store;

    slot Create(const sg_pipeline_desc& desc)
    {
        slot s = ms_store.Create();
        sg_pipeline* p = ms_store.Get(s);
        *p = sg_make_pipeline(&desc);
        Assert(p->id != SG_INVALID_ID);
        return s;
    }
    void Destroy(slot s)
    {
        if(!Exists(s))
        {
            return;
        }
        sg_pipeline& pipe = ms_store.GetUnchecked(s);
        sg_destroy_pipeline(pipe);
        ms_store.DestroyUnchecked(s);
    }
    const sg_pipeline* Get(slot s)
    {
        return ms_store.Get(s);
    }
    bool Exists(slot s)
    {
        return ms_store.Exists(s);
    }
};
