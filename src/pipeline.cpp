#include "pipeline.h"

#include "macro.h"
#include "store.h"

namespace Pipelines
{
    Store<sg_pipeline, 8> ms_store;

    slot Create(const char* name, const sg_pipeline_desc& desc)
    {
        slot s = ms_store.Create(name);
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
        sg_pipeline* p = ms_store.Get(s);
        sg_destroy_pipeline(*p);
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
    bool Exists(const char* name)
    {
        return ms_store.Exists(name);
    }
    slot Find(const char* name)
    {
        return ms_store.Find(name);
    }
    slot Find(uint32_t hash)
    {
        return ms_store.Find(hash);
    }
};
