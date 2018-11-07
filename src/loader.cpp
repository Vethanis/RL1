#include "loader.h"
#include "loaderimpl.h"

#include "buffer.h"
#include "image.h"

namespace Loaders
{
    LoaderImpl<BufferResource> ms_buf;
    LoaderImpl<ImageResource>  ms_img;

    Loader* ms_loaders[RT_Count] = 
    {
        nullptr,
        &ms_buf,
        &ms_img,
    };

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
    void DoTasks(ResourceType type, uint64_t ms)
    {
        ms_loaders[type]->DoTasks(ms);
    }
};
