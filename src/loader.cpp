#include "loader.h"
#include "loaderimpl.h"

#include "buffer.h"
#include "image.h"

LoaderImpl<BufferResource> ms_buf;
LoaderImpl<ImageResource>  ms_img;

namespace Loaders
{
    Loader* ms_loaders[RT_Count] = 
    {
        nullptr,
        &ms_buf,
        &ms_img,
    };
    uint64_t ms_budgets[RT_Count] = {0};

    slot Add(ResourceType type)
    {
        return ms_loaders[type]->Add();
    }
    void IncRef(ResourceType type, slot s)
    {
        ms_loaders[type]->IncRef(s);
    }
    void DecRef(ResourceType type, slot s)
    {
        ms_loaders[type]->DecRef(s);
    }
    Resource* Get(ResourceType type, slot s)
    {
        return ms_loaders[type]->Get(s);
    }
    bool Exists(ResourceType type, slot s)
    {
        return ms_loaders[type]->Exists(s);
    }
    void Update(ResourceType type)
    {
        ms_loaders[type]->Update();
    }
    void DoTasks(ResourceType type)
    {
        ms_loaders[type]->DoTasks(ms_budgets[type]);
    }
    void SetBudget(ResourceType type, uint64_t ms)
    {
        ms_budgets[type] = ms;
    }
    void UpdateAll()
    {
        for(uint32_t i = 0; i < RT_Count; ++i)
        {
            ResourceType rt = (ResourceType)i;
            DoTasks(rt);
            Update(rt);
        }
    }
};
